# PC: cube, the game data and cube.sh (which runs cube from that folder) in CMAKE_INSTALL_PREFIX.

# SDL2_image/SDL2_mixer may come from a private prefix (PKG_CONFIG_PATH)
set_target_properties(cube PROPERTIES INSTALL_RPATH_USE_LINK_PATH ON)

install(TARGETS cube DESTINATION .)
install(PROGRAMS deploy/pc/cube.sh DESTINATION .)

if (CUBE_DATA_DIR)
    install(DIRECTORY ${CUBE_DATA_DIR}/data ${CUBE_DATA_DIR}/packages ${CUBE_DATA_DIR}/demos DESTINATION .)
    install(DIRECTORY DESTINATION savegames)
    install(DIRECTORY DESTINATION screenshots)
else ()
    message(WARNING "CUBE_DATA_DIR not set, the game data won't be installed")
endif ()
