cmake_minimum_required(VERSION 2.8.12)

set(NAME "xslt")
project(${NAME})

set(LIBXSLT_SOURCES
    libxslt/attributes.c
    libxslt/attrvt.c
    libxslt/documents.c
    libxslt/extensions.c
    libxslt/extra.c
    libxslt/functions.c
    libxslt/imports.c
    libxslt/keys.c
    libxslt/namespaces.c
    libxslt/numbers.c
    libxslt/pattern.c
    libxslt/preproc.c
    libxslt/security.c
    libxslt/templates.c
    libxslt/transform.c
    libxslt/variables.c
    libxslt/xslt.c
    libxslt/xsltlocale.c
    libxslt/xsltutils.c
)

include_directories(
    libxslt
    "${CMAKE_SOURCE_DIR}/icu/source/common"
    "${CMAKE_SOURCE_DIR}/icu/source/i18n"
	"${CMAKE_SOURCE_DIR}/libxml2/include"
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (WIN32)

    add_definitions(-DLIBXSLT_STATIC -DLIBXML_STATIC)

else ()

    include_directories(
        ${ICU_INCLUDE_DIRS}
    )

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -O3")

endif ()

add_library(${NAME} STATIC ${LIBXSLT_INCLUDES} ${LIBXSLT_SOURCES})

add_post_build_command(xslt)

copy_library_headers_directory(xslt libxslt include/libxslt)
