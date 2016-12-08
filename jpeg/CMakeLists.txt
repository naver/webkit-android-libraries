cmake_minimum_required(VERSION 2.8.12)

set(NAME "jpeg")
project(${NAME})

set(JPEG_INCLUDES)

set(JPEG_SOURCES
    jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c
    jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c
    jcphuff.c jcprepct.c jcsample.c jctrans.c jdapimin.c jdapistd.c
    jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c jddctmgr.c jdhuff.c
    jdinput.c jdmainct.c jdmarker.c jdmaster.c jdmerge.c jdphuff.c
    jdpostct.c jdsample.c jdtrans.c jerror.c jfdctflt.c jfdctfst.c
    jfdctint.c jidctflt.c jidctfst.c jidctint.c jidctred.c jquant1.c
    jquant2.c jutils.c
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${WEBKIT_LIBRARIES_SRC}/system/core/include"
)

if (MSVC)

    set(JPEG_INCLUDES
        cderror.h
        cdjpeg.h
        jchuff.h
        jconfig.h
        jdct.h
        jdhuff.h
        jerror.h
        jinclude.h
        jmemsys.h
        jmorecfg.h
        jpegint.h
        jpeglib.h
        jversion.h
        transupp.h
    )

    set(JPEG_SOURCES
        ${JPEG_SOURCES}
        jmemansi.c
        jmemmgr.c
    )

    add_definitions(-DANDROID=1 -D_WIN32_WINNT=0x0601 -UXMD_H)

else ()

    set(JPEG_SOURCES
        ${JPEG_SOURCES}
        jmemmgr.c
        armv6_idct.s
    )

    enable_language(C ASM)
    set(ASM_OPTIONS "-x assembler-with-cpp")
    set(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}")
    set_source_files_properties(armv6_idct.s PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")

    set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -fstrict-aliasing -fprefetch-loop-arrays")
    list(APPEND JPEG_SOURCES jmem-android.c)
    add_definitions(-DAVOID_TABLES -DANDROID_TILE_BASED_DECODE -DANDROID_ARMV6_IDCT)

endif ()

add_library(${NAME} STATIC ${JPEG_INCLUDES} ${JPEG_SOURCES})

add_post_build_command(jpeg)

set(JPEG_LIBRARY_HEADERS
    jconfig.h
    jmorecfg.h
    jpeglib.h
)

copy_library_headers(jpeg "${JPEG_LIBRARY_HEADERS}" include)
