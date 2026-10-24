# Toolchain for LG NetCast 4.x TVs (openlgtv/buildroot-nc4 SDK).
#
#   export NC4_SDK=/path/to/arm-lgtv-linux-gnueabi_sdk-buildroot
#   cmake -B build-nc4 -DCMAKE_TOOLCHAIN_FILE=cmake/NC4Toolchain.cmake
#
# SDL2 with the nc4 video driver and the webos-userland nc4 stubs (liblgncopenapi,
# EGL, GLES) are expected in <sysroot>/opt/nc4.

if (NOT DEFINED ENV{NC4_SDK})
    message(FATAL_ERROR "Set NC4_SDK to the NetCast 4 buildroot SDK")
endif ()
set(NC4_SDK $ENV{NC4_SDK})

# The TVs' Cortex-A9 has NEON, the toolchain just defaults to -mfpu=vfpv3. Set the default
# flags before Buildroot's toolchain file does, so external projects (opus...) get them too.
if (NOT DEFINED CMAKE_C_FLAGS)
    set(CMAKE_C_FLAGS "-mfpu=neon -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64"
            CACHE STRING "NetCast 4 CFLAGS")
endif ()
if (NOT DEFINED CMAKE_CXX_FLAGS)
    set(CMAKE_CXX_FLAGS "-mfpu=neon -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64"
            CACHE STRING "NetCast 4 CXXFLAGS")
endif ()

include(${NC4_SDK}/share/buildroot/toolchainfile.cmake)

set(NC4_PREFIX "${CMAKE_SYSROOT}/opt/nc4")
set(ENV{PKG_CONFIG_PATH} "${NC4_PREFIX}/lib/pkgconfig")
list(APPEND CMAKE_FIND_ROOT_PATH "${NC4_PREFIX}")
# libSDL2 links liblgncopenapi.so.2, which is only on the TV
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,-rpath-link,${NC4_PREFIX}/lib")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-Wl,-rpath-link,${NC4_PREFIX}/lib")
