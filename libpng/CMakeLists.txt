cmake_minimum_required(VERSION 2.8.12)

set(NAME "png")
project(${NAME})

set(PNG_INCLUDES)

set(PNG_SOURCES
	png.c
	pngerror.c
	pngget.c
	pngmem.c
	pngpread.c
	pngread.c
	pngrio.c
	pngrtran.c
	pngrutil.c
	pngset.c
	pngtrans.c
	pngwio.c
	pngwrite.c
	pngwtran.c
	pngwutil.c
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (WIN32)

    include_directories(
        "${CMAKE_SOURCE_DIR}/zlib"
    )

    set(PNG_INCLUDES
        ${PNG_INCLUDES}
        png.h
        pngconf.h
    )

else ()

    set(PNG_SOURCES
        ${PNG_SOURCES}
        arm/arm_init.c
        arm/filter_neon_intrinsics.c
        arm/filter_neon.s
    )

    enable_language(C ASM)
    set(ASM_OPTIONS "-x assembler-with-cpp")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${ASM_OPTIONS}")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

endif ()

add_library(${NAME} STATIC ${PNG_SOURCES})

if (NOT WIN32)
    target_link_libraries(${NAME} z)
endif ()

add_post_build_command(png)

set(PNG_LIBRARY_HEADERS
    png.h
    pngconf.h
    pnglibconf.h
)

copy_library_headers(png "${PNG_LIBRARY_HEADERS}" include)
