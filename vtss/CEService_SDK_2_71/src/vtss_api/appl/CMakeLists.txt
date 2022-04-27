project(mini_app)

option(NPI_INTERFACE "Use an network interface as NPI port (run as root)" OFF)
set(NPI_INTERFACE_NAME "eth2" CACHE STRING "Name of NPI network interface")

option(IO_METHOD_UIO "Use UIO as I/O interface" ON)

if(NPI_INTERFACE)
  add_definitions(-DNPI_INTERFACE=\"${NPI_INTERFACE_NAME}\")
endif(NPI_INTERFACE)

if(IO_METHOD_UIO)
  add_definitions(-DIO_METHOD_UIO=1)
endif(IO_METHOD_UIO)

if (NOT ${VTSS_PHY_API_ONLY})
  add_executable(vtss_miniapp 
    vtss_appl.c
    vtss_appl_cli.c
    vtss_version.c
    vtss_appl_board_jr1_eval.c
    vtss_appl_board_l26_eval.c
    vtss_appl_board_serval_eval.c
    vtss_appl_board_serval_ref.c
    vtss_appl_board_jr2_ref.c
  )
else()
  add_executable(vtss_miniapp 
    vtss_appl_cu_phy.c
    vtss_appl_board_atom12_eval.c
    vtss_appl_board_tesla_eval.c
  )
endif()

install(TARGETS vtss_miniapp
        RUNTIME       DESTINATION "${BIN_INSTALL_DIR}")

target_link_libraries(vtss_miniapp vtss_api)

