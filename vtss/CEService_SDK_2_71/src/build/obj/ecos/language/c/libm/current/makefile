# eCos makefile

# This is a generated file - do not edit

export PREFIX := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/obj/ecos/install
export COMMAND_PREFIX := mipsisa32-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := UNIX
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/../eCos/packages
PACKAGE := language/c/libm/current
OBJECT_PREFIX := language_c_libm
CFLAGS := -Wall -Wpointer-arith -Wstrict-prototypes -Wundef -Woverloaded-virtual -Wno-write-strings -EL -mips32 -msoft-float -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -G0
LDFLAGS := -EL -mips32 -msoft-float -g -nostdlib -Wl,--gc-sections -Wl,-static
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := tests/vectors/acos tests/vectors/asin tests/vectors/atan tests/vectors/atan2 tests/vectors/ceil tests/vectors/cos tests/vectors/cosh tests/vectors/exp tests/vectors/fabs tests/vectors/floor tests/vectors/fmod tests/vectors/frexp tests/vectors/ldexp tests/vectors/log tests/vectors/log10 tests/vectors/modf tests/vectors/pow tests/vectors/sin tests/vectors/sinh tests/vectors/sqrt tests/vectors/tan tests/vectors/tanh

build: headers libtarget.a.stamp

LIBRARY := libtarget.a
COMPILE := src/misc/matherr.c src/misc/standard.c src/misc/compatmode.cxx src/misc/infconst.c src/double/ieee754-core/e_acos.c src/double/ieee754-core/e_asin.c src/double/ieee754-core/e_atan2.c src/double/ieee754-core/e_cosh.c src/double/ieee754-core/e_exp.c src/double/ieee754-core/e_fmod.c src/double/ieee754-core/e_log.c src/double/ieee754-core/e_log10.c src/double/ieee754-core/e_pow.c src/double/ieee754-core/e_rem_pio2.c src/double/ieee754-core/e_scalb.c src/double/ieee754-core/e_sinh.c src/double/ieee754-core/e_sqrt.c src/double/ieee754-api/w_acos.c src/double/ieee754-api/w_asin.c src/double/ieee754-api/w_atan2.c src/double/ieee754-api/w_cosh.c src/double/ieee754-api/w_exp.c src/double/ieee754-api/w_fmod.c src/double/ieee754-api/w_log.c src/double/ieee754-api/w_log10.c src/double/ieee754-api/w_pow.c src/double/ieee754-api/w_scalb.c src/double/ieee754-api/w_sinh.c src/double/ieee754-api/w_sqrt.c src/double/internal/k_cos.c src/double/internal/k_rem_pio2.c src/double/internal/k_sin.c src/double/internal/k_tan.c src/double/portable-api/s_atan.c src/double/portable-api/s_ceil.c src/double/portable-api/s_copysign.c src/double/portable-api/s_cos.c src/double/portable-api/s_expm1.c src/double/portable-api/s_fabs.c src/double/portable-api/s_finite.c src/double/portable-api/s_floor.c src/double/portable-api/s_frexp.c src/double/portable-api/s_isnan.c src/double/portable-api/s_ldexp.c src/double/portable-api/s_rint.c src/double/portable-api/s_scalbn.c src/double/portable-api/s_sin.c src/double/portable-api/s_tan.c src/double/portable-api/s_modf.c src/double/portable-api/s_tanh.c
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(if $(obj:%.o=),$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)),$(obj)))
	@cat $(foreach obj,$^,$(obj:.o=.o.d)) > $(@:.stamp=.deps)
	@touch $@

headers: mlt_headers $(PREFIX)/include/math.h $(PREFIX)/include/sys/ieeefp.h

$(PREFIX)/include/math.h: $(REPOSITORY)/$(PACKAGE)/include/math.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/sys/ieeefp.h: $(REPOSITORY)/$(PACKAGE)/include/sys/ieeefp.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

