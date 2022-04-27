
add_custom_target(tags etags `find ./include/ ./base/ ./appl/ ./boards/ ./linux_support/  -name \".*\" -prune -o -type f -print`
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} COMMENT "Generate tag file for Emacs...")

