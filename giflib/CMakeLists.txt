cmake_minimum_required(VERSION 2.8.12)

set(NAME "giflib")
project(${NAME})

set(GIF_INCLUDES)

set(GIF_SOURCES
	dgif_lib.c
	gifalloc.c
	gif_err.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DHAVE_CONFIG_H)

if (WIN32)
    
    set(GIF_INCLUDES
        ${GIF_INCLUDES}
        config.h
        gif_hash.h
        gif_lib.h
        gif_lib_private.h
    )
    
else ()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format -O3")

endif ()    

add_library(${NAME} STATIC ${GIF_INCLUDES} ${GIF_SOURCES})

if (NOT WIN32)
    set(LIBRARY_DEPS)

    #link the module like any other
    target_link_libraries(${NAME} ${LIBRARY_DEPS})
endif ()   

add_post_build_command(giflib)

set(GIFLIB_LIBRARY_HEADERS
    gif_lib.h
)

copy_library_headers(giflib "${GIFLIB_LIBRARY_HEADERS}" include/giflib)
