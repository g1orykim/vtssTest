# 
# Vitesse Switch Software.
# 
# Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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
TARGET      := ecos-arm

# General-purpose OS name
OPSYS       := ECOS

# This is the ecos HAL name - needed by the ecos make file
ECOS_HAL    := vcoreii

# The HAL startup type, and other ecos configuration
ECOS_CONF   := minimum_RAM64

# Basic modules
MODULES := ecos vtss_api vtss_appl

INCLUDES := 
DEFINES :=

export MODULES

# Defines for the modules (vtssapi) - makefile vars and C-defines
BOARD           := estax_34_ref
PHYCHIPS        := estax_34_ref

SWITCHCHIP := $(VTSS_PRODUCT_CHIP)
PLATFORM   := $(VTSS_PRODUCT_HW)
ifneq (,$(VTSS_PRODUCT_PORTS))
DEFINES += -DVTSS_OPT_PORT_COUNT=$(VTSS_PRODUCT_PORTS)
endif

# Packages together the above lines for compiler defines
DEFINES += -DVTSS_OPSYS_$(OPSYS)=1 -D_POSIX_C_SOURCE=0
DEFINES += -D$(SWITCHCHIP) -D$(PLATFORM) -DVTSS_OPT_INT_AGGR=1

# LINT project file for the SW-API
LINT_PROJ_CONFIG = $(BUILD)/make/proj_api.lnt
