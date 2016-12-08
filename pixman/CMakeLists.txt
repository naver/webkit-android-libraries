cmake_minimum_required(VERSION 2.8.12)

set(NAME "pixman")
project(${NAME})

set(PIXMAN_INCLUDES)

set(PIXMAN_SOURCES
    pixman/pixman-access-accessors.c
    pixman/pixman-access.c
    pixman/pixman-arm-neon.c
    pixman/pixman-arm-simd.c
    pixman/pixman-arm.c
    pixman/pixman-bits-image.c
    pixman/pixman-combine-float.c
    pixman/pixman-combine32.c
    pixman/pixman-conical-gradient.c
    pixman/pixman-edge-accessors.c
    pixman/pixman-edge.c
    pixman/pixman-fast-path.c
    pixman/pixman-filter.c
    pixman/pixman-general.c
    pixman/pixman-glyph.c
    pixman/pixman-gradient-walker.c
    pixman/pixman-image.c
    pixman/pixman-implementation.c
    pixman/pixman-linear-gradient.c
    pixman/pixman-matrix.c
    pixman/pixman-mips.c
    pixman/pixman-mmx.c
    pixman/pixman-noop.c
    pixman/pixman-ppc.c
    pixman/pixman-radial-gradient.c
    pixman/pixman-region16.c
    pixman/pixman-region32.c
    pixman/pixman-solid-fill.c
    pixman/pixman-timer.c
    pixman/pixman-trap.c
    pixman/pixman-utils.c
    pixman/pixman-x86.c
    pixman/pixman.c
)

if (WIN32)

    set(PIXMAN_INCLUDES
        ${PIXMAN_INCLUDES}
        config.h
        pixman/loongson-mmintrin.h
        pixman/pixman-accessor.h
        pixman/pixman-arm-common.h
        pixman/pixman-arm-neon-asm.h
        pixman/pixman-arm-simd-asm.h
        pixman/pixman-combine32.h
        pixman/pixman-compiler.h
        pixman/pixman-edge-imp.h
        pixman/pixman-inlines.h
        pixman/pixman-private.h
        pixman/pixman-version.h
        pixman/pixman.h
    )

    set(PIXMAN_SOURCES
        ${PIXMAN_SOURCES}
        pixman/pixman-sse2.c
        pixman/pixman-ssse3.c
    )

    include_directories(
        "${CMAKE_SOURCE_DIR}/pixman"
        "${CMAKE_SOURCE_DIR}/pixman/pixman"
    )

    add_definitions(-DANDROID=1 -DHAVE_CONFIG_H)

else ()

    set(ANDROID_CPU_FEATURES_PATH "${ANDROID_NDK}/sources/android/cpufeatures")
    set(CPU_FEATURE_NEON_SRCS  ${ANDROID_CPU_FEATURES_PATH}/cpu-features.c   ${ANDROID_CPU_FEATURES_PATH}/cpu-features.h)
    set_source_files_properties(CPU_FEATURE_NEON_SRCS PROPERTIES COMPILE_FLAGS "-Wno-unused-variable")

    enable_language(C ASM)
    set(ASM_OPTIONS "-x assembler-with-cpp")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${ASM_OPTIONS}")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-field-initializers -Wno-attributes -include \"limits.h\"")

    set(PIXMAN_SOURCES
        ${PIXMAN_SOURCES}
        pixman/pixman-arm-neon-asm-bilinear.s
        pixman/pixman-arm-neon-asm.s
        pixman/pixman-arm-simd-asm-scaled.s
        pixman/pixman-arm-simd-asm.s
        ${CPU_FEATURE_NEON_SRCS}
    )

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}
        pixman-extra
        pixman
        ${ANDROID_CPU_FEATURES_PATH}
    )

    add_definitions(-DHAVE_CONFIG_H -DNDEBUG -D__ARM_HAVE_NEON -DUSE_ARM_NEON -DUSE_ARM_SIMD -D_USE_MATH_DEFINES -DPIXMAN_NO_TLS)

endif ()

add_library(${NAME} STATIC ${PIXMAN_INCLUDES} ${PIXMAN_SOURCES})
