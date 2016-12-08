PROJECT(harfbuzz CXX)

set(HARFBUZZ_SOURCES
    src/hb-blob.cc
    src/hb-buffer-serialize.cc
    src/hb-buffer.cc
    src/hb-common.cc
    src/hb-face.cc
    src/hb-fallback-shape.cc
    src/hb-font.cc
    src/hb-icu.cc
    src/hb-ot-layout.cc
    src/hb-ot-map.cc
    src/hb-ot-shape-complex-arabic.cc
    src/hb-ot-shape-complex-default.cc
    src/hb-ot-shape-complex-indic-table.cc
    src/hb-ot-shape-complex-indic.cc
    src/hb-ot-shape-complex-myanmar.cc
    src/hb-ot-shape-complex-sea.cc
    src/hb-ot-shape-complex-thai.cc
    src/hb-ot-shape-fallback.cc
    src/hb-ot-shape-normalize.cc
    src/hb-ot-shape.cc
    src/hb-ot-tag.cc
    src/hb-set.cc
    src/hb-shape-plan.cc
    src/hb-shape.cc
    src/hb-shaper.cc
    src/hb-tt-font.cc
    src/hb-unicode.cc
    src/hb-warning.cc
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    src
    src/contrib
    "${CMAKE_SOURCE_DIR}/icu/source/common"
    "${CMAKE_SOURCE_DIR}/icu/source/i18n"
)

add_definitions(-DHAVE_CONFIG_H -DHB_NO_MT -DU_DISABLE_RENAMING=1)

add_library(harfbuzz STATIC ${HARFBUZZ_SOURCES})

add_post_build_command(harfbuzz)

set(HARFBUZZ_LIBRARY_HEADERS
    src/hb-blob.h
    src/hb-buffer.h
    src/hb-common.h
    src/hb-deprecated.h
    src/hb-face.h
    src/hb-font.h
    src/hb-icu.h
    src/hb-ot-layout.h
    src/hb-ot-tag.h
    src/hb-ot.h
    src/hb-set.h
    src/hb-shape-plan.h
    src/hb-shape.h
    src/hb-unicode.h
    src/hb-version.h
    src/hb.h
)

copy_library_headers(harfbuzz "${HARFBUZZ_LIBRARY_HEADERS}" include)
