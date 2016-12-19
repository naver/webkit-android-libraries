cmake_minimum_required(VERSION 2.8.12)

set(NAME "z")
project(${NAME})

set(ZLIB_INCLUDES
    crc32.h
    deflate.h
    gzguts.h
    inffast.h
    inffixed.h
    inflate.h
    inftrees.h
    mozzconf.h
    trees.h
    x86.h
    zconf.h
    zlib.h
    zutil.h
)

set(ZLIB_SOURCES
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    infback.c
    inffast.c
    inflate.c
    inftrees.c
    trees.c
    uncompr.c
    zutil.c
    x86.c
    crc_folding.c # simd
    fill_window_sse.c # simd
)

include_directories(
    "${CMAKE_SOURCE_DIR}/zlib"
)

link_directories(
    ${OUTPUT_DIR}
)

add_definitions(-DZLIB_DLL)

add_library(${NAME} SHARED ${ZLIB_INCLUDES} ${ZLIB_SOURCES})

set(LIBRARY_DEPS
    ${WIN32_SYSTEM_LIBRARIES}
)

target_link_libraries(${NAME} ${LIBRARY_DEPS})

add_post_build_command(${NAME})

set(ZLIB_LIBRARY_HEADERS
    crc32.h
    deflate.h
    gzguts.h
    inffast.h
    inffixed.h
    inflate.h
    inftrees.h
    mozzconf.h
    trees.h
    zconf.h
    zlib.h
    zutil.h
)

copy_library_headers(${NAME} "${ZLIB_LIBRARY_HEADERS}" include)
