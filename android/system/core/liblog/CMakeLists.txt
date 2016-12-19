cmake_minimum_required(VERSION 2.8.12)

set(NAME "log")
project(${NAME})

set(LOG_SOURCES
    logd_write_win32.cpp
)

include_directories(
    "${CMAKE_SOURCE_DIR}/android/system/core/include"
)

add_definitions(-DHAVE_MS_C_RUNTIME -DHAVE_WIN32_THREADS -DHAVE_WINSOCK -DOS_PATH_SEPARATOR=%27\\%27)

add_library(${NAME} SHARED ${LOG_SOURCES})

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEF:\"${CMAKE_SOURCE_DIR}/android/system/core/liblog/liblog.def\"")

target_link_libraries(${NAME} ${WIN32_SYSTEM_LIBRARIES})
