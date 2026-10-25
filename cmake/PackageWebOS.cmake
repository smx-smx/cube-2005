# LG webOS: an IPK (dist/<id>_<version>_arm.ipk) made with ares-package, installed with ares-install.
# The webOS CLI is a project-local npm package: cd tools/webos && npm ci
# The TV provides SDL2, EGL/GLES and glibc; SDL2_image and SDL2_mixer are bundled in lib/ so
# the firmware's copies (which may be old or built without Ogg) are not used.

set(WEBOS_APPINFO_ID "org.openlgtv.cube" CACHE STRING "webOS application id")
configure_file(deploy/webos/appinfo.json.in appinfo.json @ONLY)

set_target_properties(cube PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
# DT_RPATH, unlike DT_RUNPATH, is searched before the LD_LIBRARY_PATH of TV apps
target_link_options(cube PRIVATE -Wl,--disable-new-dtags)

install(TARGETS cube DESTINATION .)
install(FILES "${CMAKE_BINARY_DIR}/appinfo.json" deploy/webos/autoexec.cfg deploy/webos/icon.png
        deploy/webos/icon_large.png DESTINATION .)

if (CUBE_DATA_DIR)
    install(DIRECTORY ${CUBE_DATA_DIR}/data ${CUBE_DATA_DIR}/packages ${CUBE_DATA_DIR}/demos DESTINATION .)
    install(DIRECTORY DESTINATION savegames)
    install(DIRECTORY DESTINATION screenshots)
else ()
    message(WARNING "CUBE_DATA_DIR not set, the game data won't be installed")
endif ()

# under their SONAME, as the files themselves
foreach (lib SDL2_image SDL2_mixer)
    find_file(WEBOS_${lib}_SO lib${lib}-2.0.so.0 PATHS "${CMAKE_SYSROOT}/usr/lib" NO_DEFAULT_PATH REQUIRED)
    get_filename_component(real "${WEBOS_${lib}_SO}" REALPATH)
    install(FILES "${real}" DESTINATION lib RENAME lib${lib}-2.0.so.0)
endforeach ()

set(WEBOS_TOOLS_BIN "${CMAKE_SOURCE_DIR}/tools/webos/node_modules/.bin")
find_program(ARES_PACKAGE ares-package PATHS "${WEBOS_TOOLS_BIN}" NO_DEFAULT_PATH)
find_program(ARES_INSTALL ares-install PATHS "${WEBOS_TOOLS_BIN}" NO_DEFAULT_PATH)
find_program(ARES_LAUNCH ares-launch PATHS "${WEBOS_TOOLS_BIN}" NO_DEFAULT_PATH)
if (ARES_PACKAGE)
    set(WEBOS_STAGING "${CMAKE_BINARY_DIR}/webos-staging")
    add_custom_target(webos-package-cube
            COMMAND ${CMAKE_COMMAND} -E rm -rf "${WEBOS_STAGING}"
            COMMAND ${CMAKE_COMMAND} --install "${CMAKE_BINARY_DIR}" --prefix "${WEBOS_STAGING}" --strip
            COMMAND ${ARES_PACKAGE} "${WEBOS_STAGING}" -o "${CMAKE_SOURCE_DIR}/dist"
            DEPENDS cube
            VERBATIM)
    if (DEFINED ENV{ARES_DEVICE})
        set(ares_arguments "-d" "$ENV{ARES_DEVICE}")
    endif ()
    add_custom_target(webos-install-cube
            COMMAND ${ARES_INSTALL} "${WEBOS_APPINFO_ID}_1.0.0_arm.ipk" ${ares_arguments}
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/dist"
            DEPENDS webos-package-cube
            VERBATIM)
    add_custom_target(webos-launch-cube
            COMMAND ${ARES_LAUNCH} "${WEBOS_APPINFO_ID}" ${ares_arguments}
            DEPENDS webos-install-cube
            VERBATIM)
else ()
    message(WARNING "ares-package not found (cd tools/webos && npm ci), no webos-package-cube target")
endif ()
