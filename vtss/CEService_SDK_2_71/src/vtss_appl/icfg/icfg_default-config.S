#define xstr(s) str(s)
#define str(s) #s
        .data
	.globl	icfg_default_config
icfg_default_config:
	.incbin	xstr(CONFIGFILE)
	.globl	icfg_default_config_end
icfg_default_config_end:
	.long	0
