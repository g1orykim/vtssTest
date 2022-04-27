BINMERGE := ../../eCos/host/webstax_binmerge/webstax_binmerge.c


compile:
	gcc -g -m32 -Wall $(BINMERGE) -o webstax_binmerge

compile_256:
	gcc -g -m32 -Wall $(BINMERGE) -DALLOWED_MAXIMUM_SECTOR_SIZE=256 -o webstax_binmerge_256

dbg:
	ddd webstax_binmerge

# If not given on the command line, use this one:
REDBOOT_PATH ?= ../obj/redboot.bin
FIRMWARE_PATH ?= ../obj/*.gz

# Example #1:
# Create master file from redboot and runtime code
# with the following features:
#  - Dual Image Support
#  - Flash sector is 256 KBytes, size is 16MB, for example: M26P128 (the one on Vitesse EVB)
#  - Verbose output
#  - MAC Address = 00:01:c1:01:02:03
#  - Board ID = 1234
#  - Default bootscript
flash16_256: compile_256
	./webstax_binmerge_256 -v -sb 256 -r $(REDBOOT_PATH) -w $(FIRMWARE_PATH) -o master_flash16_256.bin -a "MAC=00:01:c1:01:02:03" -a "BOARDID=1234"

# Example #2:
# Create master file from redboot and runtime code
# with the following features:
#  - Dual Image Support
#  - Flash sector is 64 KBytes, size is 16MB, for example: MX25L128
#  - Verbose output
#  - MAC Address = 00:01:c1:01:02:03
#  - Board ID = 1234
#  - Default bootscript
flash16_64: compile
	./webstax_binmerge -v -sb 64 -sc 256 -r $(REDBOOT_PATH) -w $(FIRMWARE_PATH) -o master_flash16_64.bin -a "MAC=00:01:c1:01:02:03" -a "BOARDID=1234"

# Example #3:
# Create master file from redboot and runtime code 
# with the following features:
#  - Dual Image Support
#  - Flash sector is 128 KBytes, size is 16MB.
#  - Verbose output
#  - MAC Address = 00:01:c1:01:02:03
#  - Board ID = 1234
#  - Default bootscript
flash16_128: compile
	./webstax_binmerge -v -r $(REDBOOT_PATH) -w $(FIRMWARE_PATH) -o master_flash16_128.bin -a "MAC=00:01:c1:01:02:03" -a "BOARDID=1234"



