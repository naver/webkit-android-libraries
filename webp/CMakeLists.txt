cmake_minimum_required(VERSION 2.8.12)

set(NAME "webp")
project(${NAME})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    include
)

set(INCLUDES
    include/webp/decode.h
    include/webp/decode_vp8.h
    include/webp/encode.h
    include/webp/types.h
    src/dec/decode_vp8.h
    src/dec/vp8i.h
    src/dec/vp8li.h
    src/dec/webpi.h
    src/enc/backward_references.h
    src/enc/cost.h
    src/enc/histogram.h
    src/enc/vp8enci.h
    src/enc/vp8li.h
    src/dsp/dsp.h
    src/dsp/lossless.h
    src/dsp/yuv.h
)

set(SOURCES
    src/dec/alpha.c
    src/dec/buffer.c
    src/dec/frame.c
    src/dec/idec.c
    src/dec/io.c
    src/dec/layer.c
    src/dec/quant.c
    src/dec/tree.c
    src/dec/vp8.c
    src/dec/vp8l.c
    src/dec/webp.c
    src/enc/alpha.c
    src/enc/analysis.c
    src/enc/backward_references.c
    src/enc/config.c
    src/enc/cost.c
    src/enc/filter.c
    src/enc/frame.c
    src/enc/histogram.c
    src/enc/iterator.c
    src/enc/layer.c
    src/enc/picture.c
    src/enc/quant.c
    src/enc/syntax.c
    src/enc/tree.c
    src/enc/vp8l.c
    src/enc/webpenc.c
    src/dsp/cpu.c
    src/dsp/dec.c
    src/dsp/dec_neon.c
    src/dsp/dec_sse2.c
    src/dsp/enc.c
    src/dsp/enc_sse2.c
    src/dsp/lossless.c
    src/dsp/upsampling.c
    src/dsp/upsampling_sse2.c
    src/dsp/yuv.c
)

add_definitions(-DANDROID=1)

add_library(${NAME} STATIC ${INCLUDES} ${SOURCES})

add_post_build_command(webp)

copy_library_headers_directory(webp include/webp include/webp)
