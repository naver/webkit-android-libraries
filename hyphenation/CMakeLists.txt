cmake_minimum_required(VERSION 2.8.12)

set(NAME "hyphenation")
project(${NAME})

set(HYPHENATION_INCLUDES)

set(HYPHENATION_SOURCES
	hyphen.c
	hnjalloc.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (WIN32)
    
    include_directories(
        "${CMAKE_SOURCE_DIR}/winsock2"
        "${CMAKE_SOURCE_DIR}/android/prebuilts/ndk/current/platforms/win"
    )    
    
    set(HYPHENATION_INCLUDES
        ${HYPHENATION_INCLUDES}
        hnjalloc.h
        hyphen.h
    )
    
    add_definitions(-DANDROID=1)
    
else ()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

endif ()

add_library(${NAME} STATIC ${HYPHENATION_INCLUDES} ${HYPHENATION_SOURCES})

if (NOT WIN32)
    #link the module like any other
    target_link_libraries(${NAME} ${LIBRARY_DEPS})
endif ()

add_post_build_command(hyphenation)

set(HYPHENATION_LIBRARY_HEADERS
    hyphen.h
)

copy_library_headers(hyphenation "${HYPHENATION_LIBRARY_HEADERS}" include)
