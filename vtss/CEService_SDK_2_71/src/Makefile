#
# Target for checking out files/directories for doing automated tests (make hw tests)
#
.PHONY: all compile co_make_hw

all:
	@echo "To compile, change directory to the 'build' directory"

compile:
	$(MAKE) -C build

.PHONY: co_make_hw
co_make_hw:
	mkdir -p src/ts
	mkdir -p src/testbench
	touch .top
	cd src/testbench; \
	cvs co -P exb_jaguar_tcllib ;\
	cvs co -P exb_heathrow_tcllib ;\
	cvs co -P exb_fg_tcllib ;\
	cvs co -P exb_basic_tcllib ;\
	cvs co -P exb_sw_tcllib ;\
	cvs co -P exb_basic_post_tcllib ;\
	cvs co -P exb_jaguar_tcllib ;\
	cvs co -P sw_postsil_tcl_lib ;
	cd src/ts; \
	cvs co -P exb_webstax_test 
	$(call do_co,tools)
	echo "export EXB_DESIGN_NAME := jaguar" > src/exb_design_name.mk
