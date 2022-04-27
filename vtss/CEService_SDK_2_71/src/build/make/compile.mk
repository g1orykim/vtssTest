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
# Standard setup
.SUFFIXES: # We go by ourselves!
include $(BUILD)/config.mk

ifneq ($(filter $(VTSS_PRODUCT_NAME),10G_phy b2 estax_api 1g_phy),$(VTSS_PRODUCT_NAME))
  # E-StaX stuff
  include $(BUILD)/make/setup.mk
endif

include $(TOPABS)/build/make/paths-$(TARGET).mk
include $(BUILD)/make/doxygen.mk

.PHONY: init all clean lint copy_vtss_libs redboot build_ecos

ifneq ($(V),)
  what =
  Q    =
else
  what = @echo $1
  Q    = @
endif

# Objects
OBJECTS = $(foreach m,$(MODULES),$(OBJECTS_$m))

# Targets
TARGETS = $(foreach m,$(MODULES),$(TARGETS_$m))

# Lint
export LINTDIR=$(BUILD)/make/lint/$(subst ccache ,,$(COMPILER_PREFIX))gcc
LINT_SETUP       ?= $(LINTDIR)/std.lnt
LINT_LEVEL       ?= 2
LINT_PROJ_CONFIG ?= $(BUILD)/make/proj.lnt

# Establish VTSS_SW_OPTION_xxx for defined modules 'xxx'
UCMODULES := $(shell echo $(MODULES) | tr '[:lower:]' '[:upper:]')
DEFINES   += $(foreach mod,$(UCMODULES),-DVTSS_SW_OPTION_$(mod)=1 )
DEFINES   += -DDISABLED_MODULES="$(foreach m,$(MODULES_DISABLED),$m)"
define set_module
  MODULE_$1 := 1

endef
$(eval $(foreach mod,$(UCMODULES),$(call set_module,$(mod))))

# @arg $1 is list of modules to check for
# @arg $2 is what to return if true
# @arg $3 is what to return if false
define if-module
  $(if $(filter $1,$(MODULES)),$2,$3)
endef

# @arg $1 is the stem of a variable to expand for this platform
define BSP_Component
  $(if $(CustomBSP),$(call CustomBSP,$1),$($1_$(VTSS_PRODUCT_CHIP)) $($1_$(VTSS_PRODUCT_HW)))
endef

# Slurp module defns - main/vtss_appl *last*
ifeq ($(filter icli,$(MODULES)),icli)
include $(BUILD)/make/setup_icli.in # iCLI boilterplate rules
endif
include $(foreach m,$(filter-out main vtss_appl,$(MODULES)),$(BUILD)/make/module_$m.in)
include $(foreach m,$(filter     main vtss_appl,$(MODULES)),$(BUILD)/make/module_$m.in)

# Gotta have the path to the system include files first in the INCLUDES list, to overcome
# problems arising if a module contains header files with the same name as one of the
# standard header files.
INCLUDES := $(SYS_INCLUDES) $(INCLUDES)

# User specific compilation settings (during development)
include $(BUILD)/make/user.mk

# Targets - The user can user the M variable to select which modules he wants to have as target.
ifneq ($(M),)
  TIDY_TARGETS                       := $(foreach i,$(M),$(TIDY_FILES_$i))
  LINT_TARGETS                       := $(foreach i,$(M),$(LINT_FILES_$i))
  VTSS_CODE_STYLE_CHK_TARGETS        := $(foreach i,$(M),$(VTSS_CODE_STYLE_CHK_FILES_$i))
  VTSS_CODE_STYLE_INDENT_CHK_TARGETS := $(foreach i,$(M),$(VTSS_CODE_STYLE_CHK_FILES_TWO_INDENT_$i))
  JSLINT_TARGETS                     := $(foreach i,$(M),$(JSLINT_FILES_$i))
else
  TIDY_TARGETS                       := $(TIDY_FILES)                           $(foreach i,$(MODULES),$(TIDY_FILES_$i))
  LINT_TARGETS                       := $(LINT_FILES)                           $(foreach i,$(MODULES),$(LINT_FILES_$i))
  VTSS_CODE_STYLE_CHK_TARGETS        := $(VTSS_CODE_STYLE_CHK_FILES)            $(foreach i,$(MODULES),$(VTSS_CODE_STYLE_CHK_FILES_$i))
  VTSS_CODE_STYLE_INDENT_CHK_TARGETS := $(VTSS_CODE_STYLE_CHK_FILES_TWO_INDENT) $(foreach i,$(MODULES),$(VTSS_CODE_STYLE_CHK_FILES_TWO_INDENT_$i))
  JSLINT_TARGETS                     := $(JSLINT_FILES)                         $(foreach i,$(MODULES),$(JSLINT_FILES_$i))
endif

# Dummy
init:: ;

all: init compile link

compile: $(OBJECTS) $(LIB_FILES)

link: $(TARGETS)

copy_vtss_libs:
	$(shell mkdir -p $(BUILD)/vtss_libs)
	$(foreach lib_file,$(LIB_FILES),$(shell cp $(BUILD)/obj/$(lib_file) $(BUILD)/vtss_libs/$(lib_file)))

clean::
	-rm -f *.[oadc] *.{elf,gz,dat,txt,bin}

%.lob: %.c
	$(call what,Linting $< to lint object $@)
	$(Q)flint -w$(LINT_LEVEL) $(LINT_SETUP) $(LINT_PROJ_CONFIG) $(filter -I%,$(INCLUDES)) $(DEFINES) -u $< -oo\($@\) -zero

lint: $(LINT_TARGETS)
	$(call what,'Linting $(words $^) file(s)')
	$(Q)flint -w$(LINT_LEVEL) $(LINT_SETUP) $(LINT_PROJ_CONFIG) $(filter -I%,$(INCLUDES)) $(DEFINES) -u $^

# Create a .vtss_style file in case of style errors
code_style_chk: $(VTSS_CODE_STYLE_CHK_TARGETS)
	$(Q)vtss_c_code_style_chk.pl $(VTSS_CODE_STYLE_CHK_TARGETS)
	$(Q)vtss_c_code_style_chk.pl --indent 2 $(VTSS_CODE_STYLE_INDENT_CHK_TARGETS)

# Overwrite files themselves in case of style errors
style_inplace: $(VTSS_CODE_STYLE_CHK_TARGETS)
	$(Q)vtss_c_code_style_chk.pl --inplace $(VTSS_CODE_STYLE_CHK_TARGETS)
	$(Q)vtss_c_code_style_chk.pl --inplace --indent 2 $(VTSS_CODE_STYLE_INDENT_CHK_TARGETS)

jslint: $(JSLINT_TARGETS)
	$(call what,'Javascript linting $(words $^) file(s)')
	$(Q)perl ../make/jslint.pl $^

tidy: $(TIDY_TARGETS)
	$(call what,'Tidy $(words $^) file(s)')
	$(Q)html_tidy_chk.pl $^

ifneq ($(MAKECMDGOALS),clean)
  # Generated dependencies
  -include $(OBJECTS:.o=.d)
  # Standard rules
  include $(BUILD)/make/target-$(TARGET).mk
endif

