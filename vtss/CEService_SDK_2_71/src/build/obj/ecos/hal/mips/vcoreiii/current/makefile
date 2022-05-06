# eCos makefile

# This is a generated file - do not edit

export PREFIX := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/obj/ecos/install
export COMMAND_PREFIX := mipsisa32-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := UNIX
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/../eCos/packages
PACKAGE := hal/mips/vcoreiii/current
OBJECT_PREFIX := hal_mips_vcoreiii
CFLAGS := -Wall -Wpointer-arith -Wstrict-prototypes -Wundef -Woverloaded-virtual -Wno-write-strings -EL -mips32 -msoft-float -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -G0
LDFLAGS := -EL -mips32 -msoft-float -g -nostdlib -Wl,--gc-sections -Wl,-static
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := tests/tick tests/isr tests/m25pxx_test tests/memparam

build: headers libtarget.a.stamp

LIBRARY := libtarget.a
COMPILE := src/hal_diag.c src/platform.S src/plf_misc.c src/plf_io.c src/plf_intr.c src/ser16c550c.c
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(if $(obj:%.o=),$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)),$(obj)))
	@cat $(foreach obj,$^,$(obj:.o=.o.d)) > $(@:.stamp=.deps)
	@touch $@

$(PREFIX)/include/cyg/hal/plf_defs.inc: $(wildcard $(REPOSITORY)/$(PACKAGE)/src/plf_mk_defs.c)
	$(CC) $(CFLAGS) $(INCLUDE_PATH) -Wp,-MD,plf_defs.tmp -o plf_mk_defs.tmp -S $<
	fgrep .equ plf_mk_defs.tmp | sed s/#// > $@
	@echo $@ ": \\" > $(notdir $@).deps
	@tail -n +2 plf_defs.tmp >> $(notdir $@).deps
	@echo >> $(notdir $@).deps
	@rm plf_defs.tmp plf_mk_defs.tmp
	

headers: mlt_headers $(PREFIX)/include/cyg/hal/plf_misc.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_devcpu_pi.h $(PREFIX)/include/cyg/hal/platform.inc $(PREFIX)/include/cyg/hal/plf_intr.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_icpu_cfg.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_icpu_cfg.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_devcpu_pi.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_sys.h $(PREFIX)/include/cyg/hal/plf_io.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_common.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_asm.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_dev_gmii.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_devcpu_qs.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_arb.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_vcap_is2.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_sba.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_vcap_es0.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_fdma.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_dev10g.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_vcap_is0.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_lrn.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_uart.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_devcpu_org.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_devcpu_gcb.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_vcap_core.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_dev1g.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_dev.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_devcpu_qs.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_devnpi.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_rew.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_oqs.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_ana_l2.h $(PREFIX)/include/cyg/hal/plf_stub.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_devcpu_org.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_ana_l3_2.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_ana_cl_2.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_memctrl.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_rew.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_dev_cmn.h $(PREFIX)/include/cyg/hal/plf_cache.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_devcpu_gcb.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_ana.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_macro_ctrl.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_sch.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_common.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_fdma.h $(PREFIX)/include/cyg/hal/vcoreiii_diag.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_vcap_is1.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_uart.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_iqs.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_macro_ctrl.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_dsm.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_twi.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_dev2g5.h $(PREFIX)/include/cyg/hal/vtss_luton26_regs_twi.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_ana_ac.h $(PREFIX)/include/cyg/hal/vtss_jaguar_regs_sba.h $(PREFIX)/include/cyg/hal/hal_diag.h

$(PREFIX)/include/cyg/hal/plf_misc.h: $(REPOSITORY)/$(PACKAGE)/include/plf_misc.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_devcpu_pi.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_devcpu_pi.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/platform.inc: $(REPOSITORY)/$(PACKAGE)/include/platform.inc
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/plf_intr.h: $(REPOSITORY)/$(PACKAGE)/include/plf_intr.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_icpu_cfg.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_icpu_cfg.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_icpu_cfg.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_icpu_cfg.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_devcpu_pi.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_devcpu_pi.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_sys.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_sys.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/plf_io.h: $(REPOSITORY)/$(PACKAGE)/include/plf_io.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_common.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_common.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_asm.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_asm.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_dev_gmii.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_dev_gmii.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_devcpu_qs.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_devcpu_qs.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_arb.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_arb.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_vcap_is2.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_vcap_is2.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_sba.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_sba.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_vcap_es0.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_vcap_es0.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_fdma.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_fdma.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_dev10g.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_dev10g.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_vcap_is0.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_vcap_is0.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_lrn.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_lrn.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_uart.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_uart.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_devcpu_org.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_devcpu_org.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_devcpu_gcb.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_devcpu_gcb.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_vcap_core.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_vcap_core.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_dev1g.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_dev1g.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_dev.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_dev.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_devcpu_qs.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_devcpu_qs.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_devnpi.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_devnpi.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_rew.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_rew.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_oqs.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_oqs.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_ana_l2.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_ana_l2.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/plf_stub.h: $(REPOSITORY)/$(PACKAGE)/include/plf_stub.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_devcpu_org.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_devcpu_org.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_ana_l3_2.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_ana_l3_2.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_ana_cl_2.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_ana_cl_2.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_memctrl.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_memctrl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_rew.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_rew.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_dev_cmn.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_dev_cmn.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/plf_cache.h: $(REPOSITORY)/$(PACKAGE)/include/plf_cache.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_devcpu_gcb.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_devcpu_gcb.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_ana.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_ana.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_macro_ctrl.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_macro_ctrl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_sch.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_sch.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_common.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_common.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_fdma.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_fdma.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vcoreiii_diag.h: $(REPOSITORY)/$(PACKAGE)/include/vcoreiii_diag.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_vcap_is1.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_vcap_is1.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_uart.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_uart.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_iqs.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_iqs.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_macro_ctrl.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_macro_ctrl.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_dsm.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_dsm.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_twi.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_twi.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_dev2g5.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_dev2g5.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_luton26_regs_twi.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_luton26_regs_twi.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_ana_ac.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_ana_ac.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/vtss_jaguar_regs_sba.h: $(REPOSITORY)/$(PACKAGE)/include/vtss_jaguar_regs_sba.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/hal_diag.h: $(REPOSITORY)/$(PACKAGE)/include/hal_diag.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

