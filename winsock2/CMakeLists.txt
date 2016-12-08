cmake_minimum_required(VERSION 2.8.12)

set(NAME "winsock2")
project(${NAME})

set(WINSOCK2_SOURCES
    winsock2/Ws2_32.cpp
)

include_directories(
    winsock2
)

add_library(${NAME} STATIC ${WINSOCK2_SOURCES})

add_post_build_command(winsock2)

copy_library_headers_directory(winsock2 winsock2 include/winsock2)
