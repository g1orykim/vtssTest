# 
# Vitesse Switch Software.
# 
# Copyright (c) 2002-2009 Vitesse Semiconductor Corporation "Vitesse". All
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

# Product Chip - E_STAX_34, SPARX_II_16, SPARX_II_24, VTSS_CHIP_DAYTONA or CU_PHY
VTSS_PRODUCT_CHIP := VTSS_CHIP_DAYTONA
#VTSS_PRODUCT_CHIP := VTSS_CHIP_TALLADEGA

# Hardware Platform - BOARD_ESTAX_34_REF or BOARD_ESTAX_34_ENZO
#VTSS_PRODUCT_HW := BOARD_DAYTONA_EVAL
VTSS_PRODUCT_HW := BOARD_DAYTONA
#VTSS_PRODUCT_HW := BOARD_TALLADEGA

# For VTSS_CHIP_CU_PHY, the number of ports may be setup
#VTSS_PRODUCT_PORTS := 24

# Source makefile that setups the software package.
include $(BUILD)/make/api_linux.mk

