cmake_minimum_required(VERSION 2.8.12)

set(NAME "platforms")
project(${NAME})

set(PLATFORMS_SOURCES
    win/dirent.cpp
    win/fcntl.cpp
    win/fnmatch.cpp
    win/libgen.cpp
    win/netdb.cpp
    win/poll.cpp
    win/stdlib.cpp
    win/unistd.cpp
    win/arpa/inet.cpp
    win/linux/ashmem.cpp
    
    win/sys/mman.cpp
    win/sys/select.cpp
    win/sys/socket.cpp
    win/sys/system_properties.cpp
    win/sys/time.cpp
    
    win/win/osf.cpp
    win/win/socketpair.c
    win/win/timer_resolution_controller.cpp
    win/win/unixfd.cpp
    win/win/win32_deprecated.cpp
    win/win/ws2_shims.cpp
)

include_directories(
    win
    "${CMAKE_SOURCE_DIR}/android/system/core/include"
    "${CMAKE_SOURCE_DIR}/winsock2"
)

link_directories(${OUTPUT_DIR})

add_definitions(-DPLATFORMS_EXPORTS -DBUILDING_PLATFORMS=1 -DANDROID=1 -D_USRDLL)

add_library(${NAME} SHARED ${PLATFORMS_SOURCES})

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEF:\"${CMAKE_SOURCE_DIR}/android/prebuilts/ndk/current/platforms/platforms.def\"")

set(LIBRARY_DEPS
    liblog
    winsock2
    winmm
    ${WIN32_SYSTEM_LIBRARIES}
)

target_link_libraries(${NAME} ${LIBRARY_DEPS})

add_post_build_command(platforms)

copy_library_headers_directory(platforms win include/platforms)
