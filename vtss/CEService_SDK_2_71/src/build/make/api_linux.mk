# 
# Vitesse Switch Software.
# 
# Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
# Rights Reserved.
#
# Unpublished rights reserved under the copyright laws of the United States of
# America, other countries and international treaties. Permission to use, copy,
# store and modify, the software and its source code is granted. Permission to
# integrate into other products, disclose, transmit and distribute the software
# in an absolute machine readable format (e.g. HEX file) is also granted.  The
# source code of the software may not be disclosed, transmitted or distributed
# without the written permission of Vitesse. The software and its source code
# may only be used in products utilizing the Vitesse switch products.
#
# This copyright notice must appear in any copy, modification, disclosure,
# transmission or distribution of the software. Vitesse retains all ownership,
# copyright, trade secret and proprietary rights in the software.
#
# THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
# INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR USE AND NON-INFRINGEMENT.
# 
# This defines paths and target rules preamble/postamble
TARGET      := linux-ppc

# General-purpose OS name
OPSYS       := LINUX

# Basic modules
MODULES := vtss_api vtss_appl
export MODULES

INCLUDES := 
DEFINES :=

# Packages together the above lines for compiler defines
DEFINES += -DVTSS_OPSYS_$(OPSYS)=1
DEFINES += -D$(VTSS_PRODUCT_CHIP)

ifneq (,$(VTSS_PRODUCT_HW))
DEFINES += -D$(VTSS_PRODUCT_HW)
endif

# This allows two boards (e.g. E-StaX and B2) to be enabled
ifneq (,$(VTSS_PRODUCT_HW2))
DEFINES += -D$(VTSS_PRODUCT_HW2)
endif

# Internal aggregation for 5G ports on E-StaX-34 reference board
ifeq ($(filter BOARD_ESTAX_34_REF,$(VTSS_PRODUCT_HW)),BOARD_ESTAX_34_REF)
DEFINES += -DVTSS_OPT_INT_AGGR=1 -DVTSS_OPT_VCORE_II=0
endif

DEFINES += -DVTSS_OPT_VCORE_III=0

# Optional number of ports
ifneq (,$(VTSS_PRODUCT_PORTS))
DEFINES += -DVTSS_OPT_PORT_COUNT=$(VTSS_PRODUCT_PORTS)
endif
