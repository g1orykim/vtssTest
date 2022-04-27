        .data
	.globl	firmware_data
firmware_data:
	.incbin	"../../vtss_appl/dualcpu/firmware.bin"
	.globl	firmware_data_end
firmware_data_end:
