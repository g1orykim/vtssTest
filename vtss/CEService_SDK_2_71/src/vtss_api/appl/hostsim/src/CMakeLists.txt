add_library(venice SHARED venice.cxx base.cxx)

set_target_properties(venice PROPERTIES PUBLIC_HEADER vtss_macsec_emu_base.h)
set_target_properties(venice PROPERTIES FRAMEWORK TRUE)

install(TARGETS       venice
        LIBRARY       DESTINATION "lib"
        PUBLIC_HEADER DESTINATION "include/vtss_api")
