cmake_minimum_required(VERSION 2.8.12)

set(NAME "fontconfig")
project(${NAME})

set(FONTCONFIG_INCLUDES)

set(FONTCONFIG_SOURCES
    src/fcatomic.c
    src/fcblanks.c
    src/fccache.c
    src/fccfg.c
    src/fccharset.c
    src/fccompat.c
    src/fcdbg.c
    src/fcdefault.c
    src/fcdir.c
    src/fcfreetype.c
    src/fcfs.c
    src/fchash.c
    src/fcinit-android.c
    src/fclang.c
    src/fclist.c
    src/fcmatch.c
    src/fcmatrix.c
    src/fcname.c
    src/fcobjs.c
    src/fcpat.c
    src/fcserialize.c
    src/fcstat.c
    src/fcstr.c
    src/fcxml.c
    src/ftglue.c
    src/fcformat.c
)

include_directories(
    src
    "${CMAKE_SOURCE_DIR}/cairo/cairo-extra"
    "${CMAKE_SOURCE_DIR}/freetype/include"
    "${CMAKE_SOURCE_DIR}/icu/source/common"
    "${CMAKE_SOURCE_DIR}/icu/source/i18n"
    "${CMAKE_SOURCE_DIR}/libxml2/include"
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (WIN32)
    include_directories(
        "${CMAKE_SOURCE_DIR}/winsock2"
        "${CMAKE_SOURCE_DIR}/android/prebuilts/ndk/current/platforms/win"
    )

    set(FONTCONFIG_INCLUDES
        ${FONTCONFIG_INCLUDES}
        config-fixups.h
        config.h
        fontconfig/fcfreetype.h
        fontconfig/fcprivate.h
        fontconfig/fontconfig.h
        src/fcaliastail.h
        src/fcarch.h
        src/fcatomic.h
        src/fcftaliastail.h
        src/fcftint.h
        src/fcint.h
        src/fcmutex.h
        src/fcobjs.h
        src/fcobjshash.h
        src/fcstdint.h
        src/fcwindows.h
        src/ftglue.h
    )

    add_definitions(-DANDROID=1 -DHAVE_CONFIG_H)
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-field-initializers -Wno-attributes -Wno-missing-field-initializers -Wno-attributes")
    add_definitions(-DHAVE_CONFIG_H -DHAVE_SYS_STATFS_H -DHAVE_STDINT_H -DHAVE_UINT64_T
        -DFONTCONFIG_PATH=\"/sdcard/.fcconfig\" -DFC_CACHEDIR=\"/sdcard/.fccache\" -DFC_DEFAULT_FONTS=\"/system/fonts\")
endif ()

add_library(${NAME} STATIC ${FONTCONFIG_INCLUDES} ${FONTCONFIG_SOURCES})

add_post_build_command(fontconfig)

copy_library_headers_directory(fontconfig fontconfig include/fontconfig)
