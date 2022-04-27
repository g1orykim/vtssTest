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
.PHONY: doxygen

_A ?= @

DOXYGEN               := doxygen
DOXYGEN_OUTPUT_FOLDER := $(OBJ)/doxygen
DOXYGEN_CFG_FILE      := $(DOXYGEN_OUTPUT_FOLDER)/doxygen.cfg
# Remove -include statements from $(INCLUDES), since they'll confuse Doxygen.
INCLUDES_WITHOUT_INCLUDE = $(filter-out -include %.h,$(INCLUDES))

# The patsubsts below remove the -D and -I parts of the $(DEFINES) and $(INCLUDES) before
# presenting them to Doxygen.
doxygen: $(DOXYGEN_FILES)
	$(_A)@mkdir -p $(DOXYGEN_OUTPUT_FOLDER)
	$(_A)$(DOXYGEN) -s -g $(DOXYGEN_CFG_FILE) >/dev/null
	$(_A)sed -i -e "s|\(\bPROJECT_NAME\b.*=\).*|\1 \"Vitesse API\"|" $(DOXYGEN_CFG_FILE)
	$(_A)sed -i -e "s|\(\bINPUT\b.*=\).*|\1 $(DOXYGEN_FILES)|" $(DOXYGEN_CFG_FILE)
	$(_A)sed -i -e "s|\(\bOUTPUT_DIRECTORY\b.*=\).*|\1 $(DOXYGEN_OUTPUT_FOLDER)|" $(DOXYGEN_CFG_FILE)
	$(_A)sed -i -e "s|\(\bFULL_PATH_NAMES\b.*=\).*|\1 YES|" $(DOXYGEN_CFG_FILE)
	$(_A)sed -i -e "s|\(\bSTRIP_FROM_PATH\b.*=\).*|\1 $(OBJ)/../..|" $(DOXYGEN_CFG_FILE)
	$(_A)sed -i -e "s|\(\bOPTIMIZE_OUTPUT_FOR_C\b.*=\).*|\1 YES|" $(DOXYGEN_CFG_FILE)
	$(_A)sed -i -e "s|\(\bSOURCE_BROWSER\b.*=\).*|\1 YES|" $(DOXYGEN_CFG_FILE)
	$(_A)sed -i -e "s|\(\bDISTRIBUTE_GROUP_DOC\b.*=\).*|\1 YES|" $(DOXYGEN_CFG_FILE)
	$(_A)sed -i -e 's|\(\bPREDEFINED\b.*=\).*|\1 $(patsubst -D%,%,$(DEFINES))|' $(DOXYGEN_CFG_FILE)
	$(_A)sed -i -e 's|\(\bINCLUDE_PATH\b.*=\).*|\1 $(patsubst -I%,%,$(INCLUDES_WITHOUT_INCLUDE))|' $(DOXYGEN_CFG_FILE)
	$(_A)sed -i -e "s|\(\bPDF_HYPERLINKS\b.*=\).*|\1 YES|" $(DOXYGEN_CFG_FILE)
	$(_A)sed -i -e "s|\(\bUSE_PDFLATEX\b.*=\).*|\1 YES|" $(DOXYGEN_CFG_FILE)
	$(_A)sed -i -e "s|\(\bLATEX_BATCHMODE\b.*=\).*|\1 YES|" $(DOXYGEN_CFG_FILE)
	$(_A)$(DOXYGEN) $(DOXYGEN_CFG_FILE) 1>/dev/null
	$(_A)$(MAKE) -C $(DOXYGEN_OUTPUT_FOLDER)/latex 1>/dev/null 2>/dev/null

