# Cube (2005_08_29) for LG NetCast 4.x: SDL 2, OpenGL ES 1.1, SDL2_mixer.
#
#   export NC4_SDK=/path/to/arm-lgtv-linux-gnueabi_sdk-buildroot
#   cmake -B build-nc4 -DCMAKE_TOOLCHAIN_FILE=cmake/NC4Toolchain.cmake \
#         -DCUBE_DATA_DIR=/path/to/cube_2005_08_29_unix/cube -DCMAKE_INSTALL_PREFIX=<usb>/nc4-homebrew/cube
#   cmake --build build-nc4 && cmake --install build-nc4
cmake_minimum_required(VERSION 3.16)
project(cube C CXX)

set(CUBE_DATA_DIR "" CACHE PATH "cube directory of cube_2005_08_29_unix.tar.gz (data/, packages/...)")

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
find_package(ZLIB REQUIRED)
find_library(GLES1_LIBRARY GLESv1_CM REQUIRED)

add_library(enet STATIC
        enet/callbacks.c enet/host.c enet/list.c enet/memory.c enet/packet.c enet/peer.c enet/protocol.c
        enet/unix.c)
target_include_directories(enet PUBLIC enet/include)
# what enet's configure finds on glibc
target_compile_definitions(enet PRIVATE HAS_SOCKLEN_T HAS_POLL HAS_FCNTL HAS_MSGHDR_FLAGS HAS_GETHOSTBYNAME_R
        HAS_GETHOSTBYADDR_R)

add_executable(cube
        src/client.cpp src/clientextras.cpp src/clientgame.cpp src/clients2c.cpp src/command.cpp src/console.cpp
        src/editing.cpp src/entities.cpp src/main.cpp src/menus.cpp src/monster.cpp src/physics.cpp
        src/rendercubes.cpp src/renderextras.cpp src/rendergl.cpp src/rendermd2.cpp src/renderparticles.cpp
        src/rendertext.cpp src/rndmap.cpp src/savegamedemo.cpp src/server.cpp src/serverbrowser.cpp
        src/serverms.cpp src/serverutil.cpp src/sound.cpp src/tools.cpp src/weapon.cpp src/world.cpp
        src/worldio.cpp src/worldlight.cpp src/worldocull.cpp src/worldrender.cpp
        src/gles1compat.cpp)
target_compile_definitions(cube PRIVATE CUBE_GLES CUBE_NC4)
target_include_directories(cube PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})
# 2005 C++: string literals as char *, implicit conversions...
target_compile_options(cube PRIVATE -std=gnu++98 -fsigned-char -fpermissive -Wno-write-strings -Wno-format-security)
target_link_directories(cube PRIVATE ${SDL2_LIBRARY_DIRS} ${SDL2_IMAGE_LIBRARY_DIRS} ${SDL2_MIXER_LIBRARY_DIRS})
# the TV's app environment preloads libraries that pull in the firmware's old libstdc++
target_link_options(cube PRIVATE -static-libstdc++ -static-libgcc)
target_link_libraries(cube PRIVATE enet ${SDL2_MIXER_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_LIBRARIES}
        ${GLES1_LIBRARY} ZLIB::ZLIB m)

include(cmake/PackageNC4.cmake)
