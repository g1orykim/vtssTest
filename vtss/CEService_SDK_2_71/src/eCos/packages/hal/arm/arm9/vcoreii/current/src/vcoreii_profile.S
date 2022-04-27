        .file   "hal_arch.S"

#include <pkgconf/hal.h>
#include <pkgconf/hal_arm.h>
#ifdef CYGPKG_KERNEL
// For instrumentation options
# include <pkgconf/kernel.h>
# ifdef CYGPKG_PROFILE_GPROF
#  include <pkgconf/profile_gprof.h>
# endif
#endif                
 
	.text
#ifdef CYGPKG_PROFILE_CALLGRAPH
        .code 32
        .globl mcount
        .type mcount,function
mcount: 
        add     lr, lr, #4        // Return address - skip crud
        stmdb	sp!, {r0-r6, lr} // Save clobbered registers
        mov     r0, ip            // Salvage previous caller - in IP reg by compiler
        mov     r1, lr		  // Current caller - adjusted lr
        bl      hal_mcount        // Intermediate HAL layer
        ldmia   sp!, {r0-r6, pc} // return, restore, unwind
        nop
#endif // CYGPKG_PROFILE_CALLGRAPH

        .end
