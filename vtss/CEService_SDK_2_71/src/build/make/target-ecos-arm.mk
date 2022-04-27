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

-include $(ECOS_BUILD)/install/include/pkgconf/ecos.mak
COMPILER_PREFIX := $(ECOS_COMMAND_PREFIX)

# Suppress pointer sign mismatch warnings introduced by GCC 4.x
ECOS_GLOBAL_CFLAGS += $(if $(filter arm-eabi-,$(COMPILER_PREFIX)),-Wno-pointer-sign)

# Mimic rules.mak in ecos
ACTUAL_CFLAGS := $(filter-out -fno-rtti -Woverloaded-virtual -Winline -fvtable-gc -finit-priority,$(ECOS_GLOBAL_CFLAGS))
ACTUAL_CXXFLAGS := $(filter-out -Wstrict-prototypes -Wno-unused-but-set-variable -Wno-strict-aliasing -Wno-enum-compare -Wno-pointer-sign,$(ECOS_GLOBAL_CFLAGS))

CFLAGS		= $(INCLUDES) $(DEFINES) $(ACTUAL_CFLAGS)
CXXFLAGS        = $(INCLUDES) $(DEFINES) $(ACTUAL_CXXFLAGS)
LDFLAGS		= -nostartfiles -L$(ECOS_INSTALL)/lib -Ttarget.ld $(ECOS_GLOBAL_LDFLAGS)
XAR		= $(COMPILER_PREFIX)ar
XCC		= $(COMPILER_PREFIX)gcc
XCXX            = $(COMPILER_PREFIX)g++
XSTRIP		= $(COMPILER_PREFIX)strip
XLD		= $(XCC)
XOBJCOPY        = $(COMPILER_PREFIX)objcopy

TARGET_CPU	:= 1		# Used for firmware image target CPU type

# compile_c <o-file> <c-file> <x-flags>
define compile_c 
$(call what,Compiling $2)
$(Q)$(XCC) -c -o $1 -MD $(CFLAGS) $3 $2
endef

define compile_cxx
$(call what,Compiling $2)
$(Q)$(XCXX) -c -o $1 -MD $(CXXFLAGS) $3 $2
endef

%.cxx.o: %.cxx
	$(call compile_cxx, $@, $<)

%.o: %.c
	$(call compile_c, $@, $<)

%.bin: %.elf
	$(call what,Converting ELF to binary $@)
	$(Q)$(XOBJCOPY) -O binary $< $*.bin
