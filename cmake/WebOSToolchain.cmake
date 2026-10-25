# Toolchain for LG webOS TVs (openlgtv/buildroot-nc4 webOS SDK).
#
#   export WEBOS_SDK=/path/to/arm-webos-linux-gnueabi_sdk-buildroot
#   cmake -B build-webos -DCMAKE_TOOLCHAIN_FILE=cmake/WebOSToolchain.cmake
#
# SDL2 (with the webOS Wayland driver), SDL2_image, SDL2_mixer and the EGL/GLES stubs are
# expected in the SDK's sysroot.

if (NOT DEFINED ENV{WEBOS_SDK})
    message(FATAL_ERROR "Set WEBOS_SDK to the webOS buildroot SDK")
endif ()
set(WEBOS_SDK $ENV{WEBOS_SDK})

include(${WEBOS_SDK}/share/buildroot/toolchainfile.cmake)
