cmake_minimum_required(VERSION 2.8.12)

set(NAME "gnu-libstdc++")
project(${NAME})

if (ANDROID)
    set(LIBSTDCXX_LIBRARY_HEADERS
        include/cmath
        include/string
    )
elseif (WIN32)
    set(LIBSTDCXX_LIBRARY_HEADERS
        include/cxxabi.h
    )
endif ()

add_custom_target(${NAME} ALL echo Build target gnu-libstdc++)

add_post_build_command(${NAME})

copy_library_headers(${NAME} "${LIBSTDCXX_LIBRARY_HEADERS}" include)
