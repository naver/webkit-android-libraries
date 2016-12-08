cmake_minimum_required(VERSION 2.8.12)

set(NAME "xml2")
project(${NAME})

set(LIBXML2_INCLUDES)

set(LIBXML2_SOURCES
    c14n.c
    catalog.c
    chvalid.c
    debugXML.c
    dict.c
    DOCBparser.c
    encoding.c
    entities.c
    error.c
    globals.c
    hash.c
    HTMLparser.c
    HTMLtree.c
    legacy.c
    list.c
    nanoftp.c
    nanohttp.c
    parser.c
    parserInternals.c
    pattern.c
    relaxng.c
    SAX.c
    SAX2.c
    schematron.c
    threads.c
    tree.c
    uri.c
    valid.c
    xinclude.c
    xlink.c
    xmlIO.c
    xmlmemory.c
    xmlmodule.c
    xmlreader.c
    xmlregexp.c
    xmlsave.c
    xmlschemas.c
    xmlschemastypes.c
    xmlstring.c
    xmlunicode.c
    xmlwriter.c
    xpath.c
    xpointer.c
)

include_directories(
    include
    "${CMAKE_SOURCE_DIR}/icu/source/common"
    "${CMAKE_SOURCE_DIR}/icu/source/i18n"
)

add_definitions(-DU_DISABLE_RENAMING=1)

if (WIN32)
    include_directories(
        "${CMAKE_SOURCE_DIR}/winsock2"
        "${CMAKE_SOURCE_DIR}/android/prebuilts/ndk/current/platforms/win"
    )

    set(LIBXML2_INCLUDES
        ${LIBXML2_INCLUDES}
        config.h
        elfgcchack.h
        libxml.h
    )
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -O3")
endif ()

add_library(${NAME} STATIC ${LIBXML2_INCLUDES} ${LIBXML2_SOURCES})

add_post_build_command(xml2)

copy_library_headers_directory(xml2 include/libxml include/libxml)
