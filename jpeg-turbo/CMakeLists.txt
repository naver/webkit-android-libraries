set(JPEG_SOURCES
    jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c
    jchuff.h jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c
    jconfig.h jcparam.c jcphuff.c jcprepct.c jcsample.c jdapimin.c
    jdapistd.c jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c jdct.h
    jddctmgr.c jdhuff.c jdhuff.h jdinput.c jdmainct.c jdmarker.c
    jdmaster.c jdmerge.c jdphuff.c jdpostct.c jdsample.c jerror.c
    jerror.h jfdctflt.c jfdctfst.c jfdctint.c jidctflt.c jidctfst.c
    jidctint.c jidctred.c jinclude.h jmemmgr.c jmemnobs.c jmemsys.h
    jmorecfg.h jpegint.h jpeglib.h jpeglibmangler.h jquant1.c jquant2.c
    jutils.c jversion.h
)

enable_language(C ASM)
set(ASM_OPTIONS "-x assembler-with-cpp")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${ASM_OPTIONS}")

add_definitions(-DWITH_SIMD -DMOTION_JPEG_SUPPORTED)

if( ANDROID_ABI STREQUAL "x86" )
elseif( ANDROID_ABI STREQUAL "mips" OR ANDROID_ABI STREQUAL "armeabi" OR ANDROID_ABI STREQUAL "armeabi-v6 with VFP" )
    list(APPEND JPEG_SOURCES jsimd_none.c)
elseif( ANDROID_ABI STREQUAL "armeabi-v7a" OR ANDROID_ABI STREQUAL "armeabi-v7a with VFPV3" OR ANDROID_ABI STREQUAL "armeabi-v7a with NEON")
    list(APPEND JPEG_SOURCES simd/jsimd_arm.c)
    list(APPEND JPEG_SOURCES simd/jsimd_arm_neon.S)
else()
 message( SEND_ERROR "Unknown ANDROID_ABI=\"${ANDROID_ABI}\" is specified." )
endif()

set(LIBRARY_DEPS ${CUTILS_LIBRARIES} log)

add_library(jpeg-turbo STATIC ${JPEG_SOURCES})

#link the module like any other
target_link_libraries(jpeg-turbo ${LIBRARY_DEPS})

add_post_build_command(jpeg-turbo)

set(JPEG_LIBRARY_HEADERS
    jconfig.h
    jmorecfg.h
    jpeglib.h
    jpeglibmangler.h
)

copy_library_headers(jpeg-turbo "${JPEG_LIBRARY_HEADERS}" include)
