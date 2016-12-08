cmake_minimum_required(VERSION 2.8.12)

set(NAME "jsoncpp")
project(${NAME})

set(JSONCPP_SOURCES
    dist/jsoncpp.cpp
)

include_directories(
    dist
)

add_library(${NAME} STATIC ${JSONCPP_SOURCES})

add_post_build_command(jsoncpp)

copy_library_headers_directory(jsoncpp dist/json include/json)
