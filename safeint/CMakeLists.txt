cmake_minimum_required(VERSION 2.8.12)

set(NAME "safeint")
project(${NAME})

set(SAFEINT_INCLUDES)

set(SAFEINT_SOURCES
	safeint/safeint.cpp
)

add_definitions(-DBUILDING_SAFEINT)

if (WIN32)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})

    set(SAFEINT_INCLUDES
        ${SAFEINT_INCLUDES}
        safeint/safeint.h
        safeint/SafeInt3.hpp
    )

else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden --std=c++11")
endif ()

add_library(${NAME} STATIC ${SAFEINT_INCLUDES} ${SAFEINT_SOURCES})

if (NOT WIN32)
    #link the module like any other
    target_link_libraries(${NAME} ${LIBRARY_DEPS})
endif ()

add_post_build_command(safeint)

copy_library_headers_directory(safeint safeint include/safeint)
