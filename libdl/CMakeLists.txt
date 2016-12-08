cmake_minimum_required(VERSION 2.8.12)

set(NAME "libdl")
project(${NAME})

set(SOURCES dlfcn.c)

include_directories(
    "${CMAKE_SOURCE_DIR}/winsock2"
    "${CMAKE_SOURCE_DIR}/android/prebuilts/ndk/current/platforms/win"
)

add_definitions(-DANDROID=1)

add_library(${NAME} STATIC ${SOURCES})