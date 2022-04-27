# 
# Vitesse Switch Software.
# 
# Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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
# Derived defines

ifneq (,$(VTSS_PRODUCT_NAME))
  TURNKEY := 1
  DEFINES += -DVTSS_PERSONALITY_MANAGED=1
  INCLUDES += -I$(DIR_APPL)/main/ -include personality.h -include webstax_options.h
  ifeq ($(VTSS_PRODUCT_STACKABLE),STACKABLE)
    STACKABLE   := 1
    DEFINES += -DVTSS_PERSONALITY_STACKABLE=1
  else
    STANDALONE  := 1
  endif
endif
