cmake_minimum_required(VERSION 2.8.12)

set(NAME "pthreadnp")
project(${NAME})

set(PTHREADNP_SOURCES
    pthread.cpp
)

add_definitions(-DBUILDING_PTHREADNP)

if (WIN32)

    include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/pthreads"
        "${CMAKE_SOURCE_DIR}/android/system/core/include"
    )

    link_directories(${OUTPUT_DIR} "${CMAKE_SOURCE_DIR}/pthreadnp/pthreads-win32/lib$(PlatformArchitecture)")

    add_library(${NAME} SHARED ${PTHREADNP_SOURCES})

    string(REPLACE "/" "\\" PTHREAD_DLL "${CMAKE_SOURCE_DIR}/pthreadnp/pthreads-win32/lib$(PLATFORMARCHITECTURE)/pthreadVC2.dll")
    string(REPLACE "/" "\\" PTHREAD_DLL_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>")

    string(REPLACE "/" "\\" PTHREAD_LIB "${CMAKE_SOURCE_DIR}/pthreadnp/pthreads-win32/lib$(PLATFORMARCHITECTURE)/pthreadVC2.lib")
    string(REPLACE "/" "\\" PTHREAD_LIB_PATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>")

    add_custom_command(TARGET ${NAME}
                   PRE_BUILD
                   COMMAND xcopy /y /d ${PTHREAD_DLL} ${PTHREAD_DLL_PATH} & xcopy /y /d ${PTHREAD_LIB} ${PTHREAD_LIB_PATH})

    set(LIBRARY_DEPS
        cutils
        ndk
        pthreadVC2
        ${WIN32_SYSTEM_LIBRARIES}
    )

else ()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
    add_library(${NAME} STATIC ${PTHREADNP_SOURCES})

endif()

#link the module like any other
target_link_libraries(${NAME} ${LIBRARY_DEPS})

add_post_build_command(pthreadnp)

if (WIN32)
    copy_library_headers_directory(pthreadnp . include)
else ()
    copy_library_headers(pthreadnp pthread.h include)
endif ()
