.PHONY: top all mkdirs clean mrproper TAGS_NON_ECOS TAGS tags ID prepare_release redboot lint copy_vtss_libs style_inplace code_style_chk jslint tidy doxygen show_modules show_all_modules chks $(MODULES) update-all rs1014_chk build_ecos

# Absolute TOP (for including)
TOPABS := $(CURDIR)/..

# Relative TOP (for compiling)
TOP := ../..

# Absolute build directory
BUILD := $(TOPABS)/build

export TOP TOPABS BUILD

OBJ := $(BUILD)/obj
export OBJ

# Special configuration for api-release
ifeq ($(MAKECMDGOALS),api_release)
  # The top directory in /import/dk_software/sw_ecos_switch/ where the release files is placed.
  RELEASE_DIR ?= api
endif

include config.mk

# Make it possible to force the release script to skip generating arelease note.
# Used when releaseing multiple software packages.In this case the release note is only generated
# when the first package is released.
SKIP_REL_NOTE ?= 0

#If the user doesn't specify that we shall release a PHY standalone API, we release a standard switch API release
PHY_RELEASE_ONLY ?= "n"

DO_BRANCH ?= n

# Exporting information that is need for the release procedure.
export MODULES
export VTSS_PRODUCT_MISC

# Set default values
PASS ?= -
ALLOW_TBD ?= 1
ALLOW_FUTURE_FEATUES ?= 1

top: mkdirs all

all lint redboot code_style_chk style_inplace jslint tidy doxygen clean archive link compile build_ecos copy_vtss_libs: $(OBJ)
	$(MAKE) -C $(OBJ) -f $(BUILD)/make/compile.mk $@

# Rule to "focus" on a single module
$(MODULES): $(OBJ)
	$(MAKE) -C $(OBJ) -f $(BUILD)/make/compile.mk MODULES=$@

# Create object directory, and generate a file that causes the backup process
# not to take backup of this folder and sub-folders.
$(OBJ):
	mkdir -p $(OBJ)
	@echo "skip: *" > $(OBJ)/.nsr

clobber:
	-rm -fr $(OBJ)

mrproper: clobber
	-rm -fr config.mk TAGS $(TOPABS)/ID

TAGS:
	find .. -follow -regex .*\\.[chCH][x]* -print 2>/dev/null | grep -v -E "(build|.*#.*|/tcpip/|/lwip_tcpip/|/windows/)" | etags --members -o $@ -
	etags ../vtss_api/*/*.[ch] -o ../vtss_api/TAGS

tags:
	find .. -follow -regex .*\\.[chCH][x]* -print 2>/dev/null | grep -v -E "(build|.*#.*|/tcpip/|/lwip_tcpip/|/windows/)" | ctags --members -o $@ -
	ctags ../vtss_api/*/*.[ch] -o ../vtss_api/tags


TAGS_NON_ECOS:
	find .. -follow -regex .*\\.[chCH][x]* -print 2>/dev/null | grep -v eCos | grep -v -E "(build|.*#.*|/tcpip/|/lwip_tcpip/|/windows/)" | etags --members -o $@ -
	etags ../vtss_api/*/*.[ch] -o ../vtss_api/TAGS

ID:
	(cd $(TOPABS); mkid -s --prune=eCos/packages/net/tcpip --prune=eCos/packages/net/lwip_tcpip)

api_release:
	perl release/api_release.pl $(RELEASE_TAG) $(PREV_RELEASE_TAG) $(RELEASE_DIR) $(SKIP_REL_NOTE) $(PHY_RELEASE_ONLY)


prepare_release:
	perl release/sw_prepare_release.pl --release_num $(REL_NUM) --previous_release_tag $(PREV_RELEASE_TAG) --test_num $(TEST_NUM) --do_branch $(DO_BRANCH)

config.mk:
	@echo "You must configure your build target:"
	@find configs customer_configs -type f | egrep -v '(CVS/|\~)' | sort | perl -pe 's/(.+)/	ln -s $$1 config.mk/'
	@exit 1

show_modules:
	@echo Platform: "$(VTSS_PRODUCT_NAME)	$(VTSS_PRODUCT_NAME_GUI)	$(VTSS_PRODUCT_CHIP)	$(VTSS_PRODUCT_HW)	$(VTSS_PRODUCT_STACKABLE)"
	@echo Enabled:  $(MODULES)
	@echo Disabled: $(MODULES_DISABLED)

show_all_modules:
	@echo AllModules: $(call AllModules)

rs1014_chk:
	perl release/rs1014_chk.pl  --rel_num_to_chk $(REL_NUM) --allow_tbd $(ALLOW_TBD) --allow_future_features $(ALLOW_FUTURE_FEATURES) --password $(PASS)

chks:
	@$(MAKE) --quiet lint tidy code_style_chk jslint

style:
	@$(MAKE) --quiet code_style_chk

help:
	@echo "----- Targets -----"
	@echo "clean         : Removing all obj files except the eCos files"
	@echo "clobber       : Removing all obj files"
	@echo "mrproper      : Removing all obj files + the config.mk file"
	@echo "compile       : Compile the project"
	@echo "link          : Link the project"
	@echo "archive       : Make library archives for modules that support this"
	@echo "redboot       : Build RedBoot"
	@echo "lint          : Lint the project"
	@echo "code_style_chk: Check files for Vitesse Code Style"
	@echo "tidy          : Check HTML syntax"
	@echo "jslint        : Do java script lint"
	@echo "chks          : Runs all the four code checks above"
	@echo "style         : Alias for 'code_style_chk'"
	@echo "style_inplace : Check files for style. Modify files themselves"
	@echo "ID            : Generate id file with mkid"
	@echo "TAGS          : Generate etags"
	@echo "show_modules  : Shows the modules for the selected configuration"
	@echo "build_ecos    : Do a normal build including of eCos changes"
	@echo "doxygen       : Checks that the API is documented with doxygen comments"
	@echo ""
	@echo "rs1014_chk  PASS=<Your KT password> REL_NUM=<Release number> ALLOW_FUTURE_FEATUES=<1/0> ALLOW_TBD=<1/0>"
	@echo "              : Checks if the features defined in RS1014 matches the build system"
	@echo ""
	@echo "api_release RELEASE_TAG=<Release> PREV_RELEASE_TAG=<Previous Release> [RELEASE_DIR=<top directory>]"
	@echo "              : Target for generating a .tar file containing the source files for a API release"
	@echo ""
	@echo "prepare_release REL_NUM=<Release Number> PREV_RELEASE_TAG=<Previous Release Tag> [DO_BRANCH=<y/n>] [TEST_NUM=<test number>]"
	@echo "              : Target for preparing a software release ( Makes tag + updates copyrights )"
	@echo ""

