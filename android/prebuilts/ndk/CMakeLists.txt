cmake_minimum_required(VERSION 2.8.12)

set(NAME "ndk")
project(${NAME})

set(NDK_SOURCE_DIR current/platforms/android-14/arch-x86/usr/src)

set(NDK_SOURCES
    ${NDK_SOURCE_DIR}/dirent.cpp
    ${NDK_SOURCE_DIR}/fcntl.cpp
    ${NDK_SOURCE_DIR}/fnmatch.cpp
    ${NDK_SOURCE_DIR}/libgen.cpp
    ${NDK_SOURCE_DIR}/netdb.cpp
    ${NDK_SOURCE_DIR}/poll.cpp
    ${NDK_SOURCE_DIR}/stdlib.cpp
    ${NDK_SOURCE_DIR}/unistd.cpp
    ${NDK_SOURCE_DIR}/arpa/inet.cpp
    
    ${NDK_SOURCE_DIR}/sys/mman.cpp
    ${NDK_SOURCE_DIR}/sys/select.cpp
    ${NDK_SOURCE_DIR}/sys/socket.cpp
    ${NDK_SOURCE_DIR}/sys/system_properties.cpp
    ${NDK_SOURCE_DIR}/sys/time.cpp
    
    ${NDK_SOURCE_DIR}/win/osf.cpp
    ${NDK_SOURCE_DIR}/win/socketpair.c
    ${NDK_SOURCE_DIR}/win/timer_resolution_controller.cpp
    ${NDK_SOURCE_DIR}/win/unixfd.cpp
    ${NDK_SOURCE_DIR}/win/win32_deprecated.cpp
    ${NDK_SOURCE_DIR}/win/ws2_32.cpp
    ${NDK_SOURCE_DIR}/win/ws2_shims.cpp
)

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/current/platforms/android-14/arch-x86/usr/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/current/platforms/android-14/arch-x86/usr/include/win"
    "${CMAKE_SOURCE_DIR}/android/system/core/include"
)

link_directories(${OUTPUT_DIR})

add_definitions(-DNDK_EXPORTS -DBUILDING_NDK=1 -D_USRDLL)

add_library(${NAME} SHARED ${NDK_SOURCES})

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEF:\"${CMAKE_SOURCE_DIR}/android/prebuilts/ndk/ndk.def\"")

set(LIBRARY_DEPS
    log
    winmm
    ${WIN32_SYSTEM_LIBRARIES}
)

target_link_libraries(${NAME} ${LIBRARY_DEPS})

add_post_build_command(${NAME})

copy_library_headers_directory(${NAME} current/platforms/android-14/arch-x86/usr/include include/ndk)
