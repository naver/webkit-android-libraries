cmake_minimum_required(VERSION 2.8.12)

set(NAME "cutils")
project(${NAME})

if (WIN32)

    include_directories(
        "${CMAKE_SOURCE_DIR}/android/system/core/include"
        "${CMAKE_SOURCE_DIR}/android/system/core/include/cutils"
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${CMAKE_SOURCE_DIR}/winsock2"
        "${CMAKE_SOURCE_DIR}/android/prebuilts/ndk/current/platforms/win"
    )

    set(CUTILS_SOURCES
        ashmem-dev.c
        ashmem-host.c
        atomic.c
        hashmap.c
        threads.c
        uio.c
    )

    add_definitions(-DANDROID=1 -DHAVE_MS_C_RUNTIME -DHAVE_WIN32_THREADS -DHAVE_WINSOCK -DOS_PATH_SEPARATOR=%27\\%27)

    add_library(${NAME} STATIC ${CUTILS_SOURCES})

else ()

    set(CUTILS_SOURCES
        ashmem-dev.c
        atomic.c
        hashmap.c
        threads.c
    )

    enable_language(C ASM)
    set(ASM_OPTIONS "-x assembler-with-cpp")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${ASM_OPTIONS}")

    set_source_files_properties(arch-arm/memset32.s PROPERTIES COMPILE_FLAGS "${CMAKE_ASM_FLAGS}")

    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
         $(KERNEL_HEADERS)
         ../include
    )

    list(APPEND CUTILS_SOURCES atomic-android-sh.c log.c properties.c sched_policy.c qtaguid.c arch-arm/memset32.s)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format -O3")
    add_definitions(-DANDROID_SMP=1 -DHAVE_SCHED_H -DHAVE_PTHREADS -DHAVE_SYS_UIO_H)

    set(LIBRARY_DEPS)
    add_library(${NAME} STATIC ${CUTILS_SOURCES})

    #link the module like any other
    target_link_libraries(${NAME} ${LIBRARY_DEPS})

endif()

add_post_build_command(cutils)

copy_library_headers_directory(cutils ../include/cutils include/cutils)
