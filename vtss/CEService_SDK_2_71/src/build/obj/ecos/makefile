# eCos makefile

# This is a generated file - do not edit

export HOST := UNIX
export PREFIX := /home/ubuntu160432/vtss/CEService_SDK_2_71/src/build/obj/ecos/install
export COMMAND_PREFIX := mipsisa32-elf-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export AR := $(COMMAND_PREFIX)ar

.PHONY: default build clean tests headers

build: headers $(PREFIX)/include/pkgconf/ecos.mak
	$(MAKE) -r -C hal/mips/vcoreiii/current $(PREFIX)/include/cyg/hal/plf_defs.inc
	$(MAKE) -r -C services/memalloc/common/current heapgeninc.tcl
	$(MAKE) -r -C services/memalloc/common/current heaps.cxx
	$(MAKE) -r -C hal/mips/arch/current $@
	$(MAKE) -r -C hal/mips/mips32/current $@
	$(MAKE) -r -C hal/mips/vcoreiii/current $@
	$(MAKE) -r -C devs/flash/spi/m25pxx/current $@
	$(MAKE) -r -C devs/spi/mips/vcoreiii/current $@
	$(MAKE) -r -C devs/serial/generic/16x5x/current $@
	$(MAKE) -r -C devs/serial/mips/vcoreiii/current $@
	$(MAKE) -r -C io/spi/current $@
	$(MAKE) -r -C devs/i2c/mips/vcoreiii/current $@
	$(MAKE) -r -C io/flash/current $@
	$(MAKE) -r -C hal/common/current $@
	$(MAKE) -r -C io/common/current $@
	$(MAKE) -r -C infra/current $@
	$(MAKE) -r -C error/current $@
	$(MAKE) -r -C isoinfra/current $@
	$(MAKE) -r -C devs/eth/mips/vcoreiii/current $@
	$(MAKE) -r -C io/serial/current $@
	$(MAKE) -r -C io/i2c/current $@
	$(MAKE) -r -C kernel/current $@
	$(MAKE) -r -C language/c/libc/string/current $@
	$(MAKE) -r -C language/c/libc/time/current $@
	$(MAKE) -r -C language/c/libc/stdlib/current $@
	$(MAKE) -r -C language/c/libc/stdio/current $@
	$(MAKE) -r -C language/c/libc/i18n/current $@
	$(MAKE) -r -C services/memalloc/common/current $@
	$(MAKE) -r -C net/bsd_tcpip/current $@
	$(MAKE) -r -C net/common/current $@
	$(MAKE) -r -C io/eth/current $@
	$(MAKE) -r -C io/fileio/current $@
	$(MAKE) -r -C services/crc/current $@
	$(MAKE) -r -C net/athttpd/current $@
	$(MAKE) -r -C net/sntp/current $@
	$(MAKE) -r -C io/wallclock/current $@
	$(MAKE) -r -C net/snmp/lib/current $@
	$(MAKE) -r -C compat/posix/current $@
	$(MAKE) -r -C language/c/libc/startup/current $@
	$(MAKE) -r -C net/snmp/agent/current $@
	$(MAKE) -r -C net/openssl/current $@
	$(MAKE) -r -C net/ns/dns/current $@
	$(MAKE) -r -C language/c/libm/current $@
	$(MAKE) -r -C services/cpuload/current $@
	$(MAKE) -r -C services/threadload/current $@
	$(MAKE) -r -C language/c/libc/common/current $@
	$(MAKE) -r -C services/compress/zlib/current $@
	$(MAKE) -r -C fs/ram/current $@
	$(MAKE) -r -C hal/common/current $(PREFIX)/lib/extras.o
	$(MAKE) -r -C hal/mips/arch/current $(PREFIX)/lib/vectors.o
	$(MAKE) -r -C hal/mips/mips32/current $(PREFIX)/lib/target.ld
	@echo $@ finished

clean:
	$(MAKE) -r -C hal/mips/arch/current $@
	$(MAKE) -r -C hal/mips/mips32/current $@
	$(MAKE) -r -C hal/mips/vcoreiii/current $@
	$(MAKE) -r -C devs/flash/spi/m25pxx/current $@
	$(MAKE) -r -C devs/spi/mips/vcoreiii/current $@
	$(MAKE) -r -C devs/serial/generic/16x5x/current $@
	$(MAKE) -r -C devs/serial/mips/vcoreiii/current $@
	$(MAKE) -r -C io/spi/current $@
	$(MAKE) -r -C devs/i2c/mips/vcoreiii/current $@
	$(MAKE) -r -C io/flash/current $@
	$(MAKE) -r -C hal/common/current $@
	$(MAKE) -r -C io/common/current $@
	$(MAKE) -r -C infra/current $@
	$(MAKE) -r -C error/current $@
	$(MAKE) -r -C isoinfra/current $@
	$(MAKE) -r -C devs/eth/mips/vcoreiii/current $@
	$(MAKE) -r -C io/serial/current $@
	$(MAKE) -r -C io/i2c/current $@
	$(MAKE) -r -C kernel/current $@
	$(MAKE) -r -C language/c/libc/string/current $@
	$(MAKE) -r -C language/c/libc/time/current $@
	$(MAKE) -r -C language/c/libc/stdlib/current $@
	$(MAKE) -r -C language/c/libc/stdio/current $@
	$(MAKE) -r -C language/c/libc/i18n/current $@
	$(MAKE) -r -C services/memalloc/common/current $@
	$(MAKE) -r -C net/bsd_tcpip/current $@
	$(MAKE) -r -C net/common/current $@
	$(MAKE) -r -C io/eth/current $@
	$(MAKE) -r -C io/fileio/current $@
	$(MAKE) -r -C services/crc/current $@
	$(MAKE) -r -C net/athttpd/current $@
	$(MAKE) -r -C net/sntp/current $@
	$(MAKE) -r -C io/wallclock/current $@
	$(MAKE) -r -C net/snmp/lib/current $@
	$(MAKE) -r -C compat/posix/current $@
	$(MAKE) -r -C language/c/libc/startup/current $@
	$(MAKE) -r -C net/snmp/agent/current $@
	$(MAKE) -r -C net/openssl/current $@
	$(MAKE) -r -C net/ns/dns/current $@
	$(MAKE) -r -C language/c/libm/current $@
	$(MAKE) -r -C services/cpuload/current $@
	$(MAKE) -r -C services/threadload/current $@
	$(MAKE) -r -C language/c/libc/common/current $@
	$(MAKE) -r -C services/compress/zlib/current $@
	$(MAKE) -r -C fs/ram/current $@
	@echo $@ finished

tests: build
	$(MAKE) -r -C hal/mips/arch/current $@
	$(MAKE) -r -C hal/mips/mips32/current $@
	$(MAKE) -r -C hal/mips/vcoreiii/current $@
	$(MAKE) -r -C devs/flash/spi/m25pxx/current $@
	$(MAKE) -r -C devs/spi/mips/vcoreiii/current $@
	$(MAKE) -r -C devs/serial/generic/16x5x/current $@
	$(MAKE) -r -C devs/serial/mips/vcoreiii/current $@
	$(MAKE) -r -C io/spi/current $@
	$(MAKE) -r -C devs/i2c/mips/vcoreiii/current $@
	$(MAKE) -r -C io/flash/current $@
	$(MAKE) -r -C hal/common/current $@
	$(MAKE) -r -C io/common/current $@
	$(MAKE) -r -C infra/current $@
	$(MAKE) -r -C error/current $@
	$(MAKE) -r -C isoinfra/current $@
	$(MAKE) -r -C devs/eth/mips/vcoreiii/current $@
	$(MAKE) -r -C io/serial/current $@
	$(MAKE) -r -C io/i2c/current $@
	$(MAKE) -r -C kernel/current $@
	$(MAKE) -r -C language/c/libc/string/current $@
	$(MAKE) -r -C language/c/libc/time/current $@
	$(MAKE) -r -C language/c/libc/stdlib/current $@
	$(MAKE) -r -C language/c/libc/stdio/current $@
	$(MAKE) -r -C language/c/libc/i18n/current $@
	$(MAKE) -r -C services/memalloc/common/current $@
	$(MAKE) -r -C net/bsd_tcpip/current $@
	$(MAKE) -r -C net/common/current $@
	$(MAKE) -r -C io/eth/current $@
	$(MAKE) -r -C io/fileio/current $@
	$(MAKE) -r -C services/crc/current $@
	$(MAKE) -r -C net/athttpd/current $@
	$(MAKE) -r -C net/sntp/current $@
	$(MAKE) -r -C io/wallclock/current $@
	$(MAKE) -r -C net/snmp/lib/current $@
	$(MAKE) -r -C compat/posix/current $@
	$(MAKE) -r -C language/c/libc/startup/current $@
	$(MAKE) -r -C net/snmp/agent/current $@
	$(MAKE) -r -C net/openssl/current $@
	$(MAKE) -r -C net/ns/dns/current $@
	$(MAKE) -r -C language/c/libm/current $@
	$(MAKE) -r -C services/cpuload/current $@
	$(MAKE) -r -C services/threadload/current $@
	$(MAKE) -r -C language/c/libc/common/current $@
	$(MAKE) -r -C services/compress/zlib/current $@
	$(MAKE) -r -C fs/ram/current $@
	@echo $@ finished

headers:
	$(MAKE) -r -C hal/mips/arch/current $@
	$(MAKE) -r -C hal/mips/mips32/current $@
	$(MAKE) -r -C hal/mips/vcoreiii/current $@
	$(MAKE) -r -C devs/flash/spi/m25pxx/current $@
	$(MAKE) -r -C devs/spi/mips/vcoreiii/current $@
	$(MAKE) -r -C devs/serial/generic/16x5x/current $@
	$(MAKE) -r -C devs/serial/mips/vcoreiii/current $@
	$(MAKE) -r -C io/spi/current $@
	$(MAKE) -r -C devs/i2c/mips/vcoreiii/current $@
	$(MAKE) -r -C io/flash/current $@
	$(MAKE) -r -C hal/common/current $@
	$(MAKE) -r -C io/common/current $@
	$(MAKE) -r -C infra/current $@
	$(MAKE) -r -C error/current $@
	$(MAKE) -r -C isoinfra/current $@
	$(MAKE) -r -C devs/eth/mips/vcoreiii/current $@
	$(MAKE) -r -C io/serial/current $@
	$(MAKE) -r -C io/i2c/current $@
	$(MAKE) -r -C kernel/current $@
	$(MAKE) -r -C language/c/libc/string/current $@
	$(MAKE) -r -C language/c/libc/time/current $@
	$(MAKE) -r -C language/c/libc/stdlib/current $@
	$(MAKE) -r -C language/c/libc/stdio/current $@
	$(MAKE) -r -C language/c/libc/i18n/current $@
	$(MAKE) -r -C services/memalloc/common/current $@
	$(MAKE) -r -C net/bsd_tcpip/current $@
	$(MAKE) -r -C net/common/current $@
	$(MAKE) -r -C io/eth/current $@
	$(MAKE) -r -C io/fileio/current $@
	$(MAKE) -r -C services/crc/current $@
	$(MAKE) -r -C net/athttpd/current $@
	$(MAKE) -r -C net/sntp/current $@
	$(MAKE) -r -C io/wallclock/current $@
	$(MAKE) -r -C net/snmp/lib/current $@
	$(MAKE) -r -C compat/posix/current $@
	$(MAKE) -r -C language/c/libc/startup/current $@
	$(MAKE) -r -C net/snmp/agent/current $@
	$(MAKE) -r -C net/openssl/current $@
	$(MAKE) -r -C net/ns/dns/current $@
	$(MAKE) -r -C language/c/libm/current $@
	$(MAKE) -r -C services/cpuload/current $@
	$(MAKE) -r -C services/threadload/current $@
	$(MAKE) -r -C language/c/libc/common/current $@
	$(MAKE) -r -C services/compress/zlib/current $@
	$(MAKE) -r -C fs/ram/current $@
	@echo $@ finished

$(PREFIX)/include/pkgconf/ecos.mak: makefile
	@echo 'ECOS_GLOBAL_CFLAGS = -Wall -Wpointer-arith -Wstrict-prototypes -Wundef -Woverloaded-virtual -Wno-write-strings -EL -mips32 -msoft-float -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -G0' > $@
	@echo 'ECOS_GLOBAL_LDFLAGS = -EL -mips32 -msoft-float -g -nostdlib -Wl,--gc-sections -Wl,-static' >> $@
	@echo 'ECOS_COMMAND_PREFIX = $(COMMAND_PREFIX)' >> $@

