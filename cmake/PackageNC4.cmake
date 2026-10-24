# LG NetCast 4.x: a self-contained folder for <usb>/nc4-homebrew/cube, started with
# webos-userland's "nc4app run cube/cube.sh".

set_target_properties(cube PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
# DT_RPATH, unlike DT_RUNPATH, is searched before the LD_LIBRARY_PATH of TV apps
target_link_options(cube PRIVATE -Wl,--disable-new-dtags)

install(TARGETS cube DESTINATION .)
install(PROGRAMS deploy/nc4/cube.sh DESTINATION .)
install(FILES deploy/nc4/autoexec.cfg DESTINATION .)

if (CUBE_DATA_DIR)
    install(DIRECTORY ${CUBE_DATA_DIR}/data ${CUBE_DATA_DIR}/packages ${CUBE_DATA_DIR}/demos DESTINATION .)
    install(DIRECTORY DESTINATION savegames)
    install(DIRECTORY DESTINATION screenshots)
else ()
    message(WARNING "CUBE_DATA_DIR not set, the game data won't be installed")
endif ()

# Bundle every shared library the TV doesn't have, under their SONAME (the stick is FAT)
install(CODE "
    set(NC4_OBJDUMP \"${CMAKE_OBJDUMP}\")
    set(NC4_SYSROOT \"${CMAKE_SYSROOT}\")
    set(NC4_BUILD_DIR \"${CMAKE_BINARY_DIR}\")
    set(NC4_EXECUTABLE \"\${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:cube>\")
    set(NC4_LIBDIR \"\${CMAKE_INSTALL_PREFIX}/lib\")
    include(\"${CMAKE_SOURCE_DIR}/cmake/NC4BundleLibs.cmake\")
")
