cmake_minimum_required(VERSION 2.8.12)

set(NAME "cairo")
project(${NAME})

set(CAIRO_INCLUDES)

set(CAIRO_SOURCES
	src/cairo-analysis-surface.c
	src/cairo-arc.c
	src/cairo-array.c
	src/cairo-atomic.c
	src/cairo-base64-stream.c
	src/cairo-base85-stream.c
	src/cairo-bentley-ottmann-rectangular.c
	src/cairo-bentley-ottmann-rectilinear.c
	src/cairo-bentley-ottmann.c
	src/cairo-botor-scan-converter.c
	src/cairo-boxes-intersect.c
	src/cairo-boxes.c
	src/cairo.c
	src/cairo-cache.c
	src/cairo-cff-subset.c
	src/cairo-clip-boxes.c
	src/cairo-clip-polygon.c
	src/cairo-clip-region.c
	src/cairo-clip-surface.c
	src/cairo-clip-tor-scan-converter.c
	src/cairo-clip.c
	src/cairo-color.c
	src/cairo-composite-rectangles.c
	src/cairo-compositor.c
	src/cairo-contour.c
	src/cairo-damage.c
	src/cairo-debug.c
	src/cairo-default-context.c
	src/cairo-deflate-stream.c
	src/cairo-device.c
	src/cairo-egl-context.c
	src/cairo-error.c
	src/cairo-fallback-compositor.c
	src/cairo-fixed.c
	src/cairo-font-face-twin-data.c
	src/cairo-font-face-twin.c
	src/cairo-font-face.c
	src/cairo-font-options.c
	src/cairo-freed-pool.c
	src/cairo-freelist.c
	src/cairo-gl-composite.c
	src/cairo-gl-device.c
	src/cairo-gl-dispatch.c
	src/cairo-gl-glyphs.c
	src/cairo-gl-gradient.c
	src/cairo-gl-info.c
	src/cairo-gl-msaa-compositor.c
	src/cairo-gl-operand.c
	src/cairo-gl-shaders.c
	src/cairo-gl-source.c
	src/cairo-gl-spans-compositor.c
	src/cairo-gl-surface.c
	src/cairo-gl-traps-compositor.c
	src/cairo-gstate.c
	src/cairo-hash.c
	src/cairo-hull.c
	src/cairo-image-compositor.c
	src/cairo-image-info.c
	src/cairo-image-source.c
	src/cairo-image-surface.c
	src/cairo-lzw.c
	src/cairo-line.c
	src/cairo-mask-compositor.c
	src/cairo-matrix.c
	src/cairo-mempool.c
	src/cairo-mesh-pattern-rasterizer.c
	src/cairo-misc.c
	src/cairo-mono-scan-converter.c
	src/cairo-mutex.c
	src/cairo-no-compositor.c
	src/cairo-observer.c
	src/cairo-output-stream.c
	src/cairo-paginated-surface.c
	src/cairo-path-bounds.c
	src/cairo-path-fill.c
	src/cairo-path-fixed.c
	src/cairo-path-in-fill.c
	src/cairo-path-stroke-boxes.c
	src/cairo-path-stroke-polygon.c
	src/cairo-path-stroke-traps.c
	src/cairo-path-stroke-tristrip.c
	src/cairo-path-stroke.c
	src/cairo-path.c
	src/cairo-pattern.c
	src/cairo-pen.c
	src/cairo-pdf-operators.c
	src/cairo-pdf-shading.c
	src/cairo-pdf-surface.c
	src/cairo-png.c
	src/cairo-polygon-intersect.c
	src/cairo-polygon-reduce.c
	src/cairo-polygon.c
	src/cairo-raster-source-pattern.c
	src/cairo-recording-surface.c
	src/cairo-rectangle.c
	src/cairo-rectangular-scan-converter.c
	src/cairo-region.c
	src/cairo-rtree.c
	src/cairo-scaled-font-subsets.c
	src/cairo-scaled-font.c
	src/cairo-shape-mask-compositor.c
	src/cairo-slope.c
	src/cairo-spans-compositor.c
	src/cairo-spans.c
	src/cairo-spline.c
	src/cairo-stroke-dash.c
	src/cairo-stroke-style.c
	src/cairo-surface-clipper.c
	src/cairo-surface-fallback.c
	src/cairo-surface-observer.c
	src/cairo-surface-offset.c
	src/cairo-surface-snapshot.c
	src/cairo-surface-subsurface.c
	src/cairo-surface-wrapper.c
	src/cairo-surface.c
	src/cairo-time.c
	src/cairo-tor-scan-converter.c
	src/cairo-tor22-scan-converter.c
	src/cairo-toy-font-face.c
	src/cairo-traps-compositor.c
	src/cairo-traps.c
	src/cairo-tristrip.c
	src/cairo-truetype-subset.c
	src/cairo-type1-fallback.c
	src/cairo-type1-glyph-names.c
	src/cairo-type1-subset.c
	src/cairo-type3-glyph-surface.c
	src/cairo-unicode.c
	src/cairo-user-font.c
	src/cairo-version.c
	src/cairo-wideint.c
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
	cairo-extra
	src
    "${CMAKE_SOURCE_DIR}"
	"${CMAKE_SOURCE_DIR}/pixman/pixman"
	"${CMAKE_SOURCE_DIR}/fontconfig"
	"${CMAKE_SOURCE_DIR}/android/system/core/include"
	"${CMAKE_SOURCE_DIR}/freetype/include"
	"${CMAKE_SOURCE_DIR}/libpng"
	"${CMAKE_SOURCE_DIR}/giflib"
	"${CMAKE_SOURCE_DIR}/webp"
)

if (WIN32)

    include_directories(
        "${ANGLE_LIBRARY_DIR}/include"
        "${CMAKE_SOURCE_DIR}/android/prebuilts/ndk/current/platforms/win"
        "${CMAKE_SOURCE_DIR}/jpeg"
        "${CMAKE_SOURCE_DIR}/winsock2"
        "${CMAKE_SOURCE_DIR}/winsock2/winsock2"
        "${CMAKE_SOURCE_DIR}/zlib"
    )

    set(CAIRO_INCLUDES
        ${CAIRO_INCLUDES}
        boilerplate/cairo-boilerplate-private.h
        boilerplate/cairo-boilerplate-system.h
        boilerplate/cairo-boilerplate.h
        cairo-version.h
        config.h
        src/cairo-analysis-surface-private.h
        src/cairo-arc-private.h
        src/cairo-array-private.h
        src/cairo-atomic-private.h
        src/cairo-backend-private.h
        src/cairo-box-inline.h
        src/cairo-boxes-private.h
        src/cairo-cache-private.h
        src/cairo-clip-inline.h
        src/cairo-clip-private.h
        src/cairo-combsort-inline.h
        src/cairo-compiler-private.h
        src/cairo-composite-rectangles-private.h
        src/cairo-compositor-private.h
        src/cairo-contour-inline.h
        src/cairo-contour-private.h
        src/cairo-damage-private.h
        src/cairo-default-context-private.h
        src/cairo-deprecated.h
        src/cairo-device-private.h
        src/cairo-drm.h
        src/cairo-error-inline.h
        src/cairo-error-private.h
        src/cairo-features.h
        src/cairo-fixed-private.h
        src/cairo-fixed-type-private.h
        src/cairo-fontconfig-private.h
        src/cairo-freed-pool-private.h
        src/cairo-freelist-private.h
        src/cairo-freelist-type-private.h
        src/cairo-ft-private.h
        src/cairo-ft.h
        src/cairo-gl-dispatch-private.h
        src/cairo-gl-ext-def-private.h
        src/cairo-gl-gradient-private.h
        src/cairo-gl-private.h
        src/cairo-gl.h
        src/cairo-gstate-private.h
        src/cairo-hash-private.h
        src/cairo-image-info-private.h
        src/cairo-image-surface-inline.h
        src/cairo-image-surface-private.h
        src/cairo-line-inline.h
        src/cairo-line-private.h
        src/cairo-list-inline.h
        src/cairo-list-private.h
        src/cairo-malloc-private.h
        src/cairo-mempool-private.h
        src/cairo-mutex-impl-private.h
        src/cairo-mutex-list-private.h
        src/cairo-mutex-private.h
        src/cairo-mutex-type-private.h
        src/cairo-output-stream-private.h
        src/cairo-paginated-private.h
        src/cairo-paginated-surface-private.h
        src/cairo-path-fixed-private.h
        src/cairo-path-private.h
        src/cairo-pattern-inline.h
        src/cairo-pattern-private.h
        src/cairo-pixman-private.h
        src/cairo-private.h
        src/cairo-recording-surface-inline.h
        src/cairo-recording-surface-private.h
        src/cairo-reference-count-private.h
        src/cairo-region-private.h
        src/cairo-rtree-private.h
        src/cairo-scaled-font-private.h
        src/cairo-scaled-font-subsets-private.h
        src/cairo-skia.h
        src/cairo-slope-private.h
        src/cairo-spans-compositor-private.h
        src/cairo-spans-private.h
        src/cairo-stroke-dash-private.h
        src/cairo-supported-features.h
        src/cairo-surface-backend-private.h
        src/cairo-surface-clipper-private.h
        src/cairo-surface-fallback-private.h
        src/cairo-surface-inline.h
        src/cairo-surface-observer-inline.h
        src/cairo-surface-observer-private.h
        src/cairo-surface-offset-private.h
        src/cairo-surface-private.h
        src/cairo-surface-snapshot-inline.h
        src/cairo-surface-snapshot-private.h
        src/cairo-surface-subsurface-inline.h
        src/cairo-surface-subsurface-private.h
        src/cairo-surface-wrapper-private.h
        src/cairo-time-private.h
        src/cairo-traps-private.h
        src/cairo-tristrip-private.h
        src/cairo-truetype-subset-private.h
        src/cairo-type1-private.h
        src/cairo-type3-glyph-surface-private.h
        src/cairo-types-private.h
        src/cairo-user-font-private.h
        src/cairo-version.h
        src/cairo-wideint-private.h
        src/cairo-wideint-type-private.h
        src/cairo-win32.h
        src/cairo.h
        src/cairoint.h
        src/win32/cairo-dwrite-private.h
        src/win32/cairo-win32-private.h
    )

    # add windows dependent sources.
    set(CAIRO_SOURCES
        ${CAIRO_SOURCES}
        src/win32/cairo-dwrite-font.cpp
        src/win32/cairo-win32-debug.c
        src/win32/cairo-win32-device.c
        src/win32/cairo-win32-display-surface.c
        src/win32/cairo-win32-font.c
        src/win32/cairo-win32-gdi-compositor.c
        src/win32/cairo-win32-printing-surface.c
        src/win32/cairo-win32-surface.c
        src/win32/cairo-win32-system.c
        boilerplate/cairo-boilerplate-constructors.c
        boilerplate/cairo-boilerplate-egl.c
        boilerplate/cairo-boilerplate-system.c
        boilerplate/cairo-boilerplate-win32-printing.c
        boilerplate/cairo-boilerplate-win32.c
        boilerplate/cairo-boilerplate.c
    )

    add_definitions(-DCAIRO_EXPORTS -DANDROID=1 -DCAIRO_HAS_DLSYM=1 -DHAVE_STDINT_H=1 -DHAVE_UINT64_T=1 -D_WIN32_WINNT=0x0601)

    add_library(${NAME} SHARED ${CAIRO_INCLUDES} ${CAIRO_SOURCES})

    set(LIBRARY_DEPS
        pixman
        png
        zlib
        libdl
        msimg32
        ${WIN32_SYSTEM_LIBRARIES}
    )

else ()

    include_directories(
        "${CMAKE_SOURCE_DIR}/jpeg-turbo"
    )

    # add android dependent sources.
    set(CAIRO_SOURCES
        ${CAIRO_SOURCES}
        src/cairo-ft-font.c
    )

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wno-missing-field-initializers -Wno-attributes")
    add_definitions(-DHAVE_CONFIG_H -DNDEBUG -D__ARM_HAVE_NEON -DHAVE_STDINT_H -DHAVE_ENDIAN_H -DHAVE_UINT64_T -DCAIRO_HAS_DLSYM=1)

    add_library(${NAME} STATIC ${CAIRO_INCLUDES} ${CAIRO_SOURCES})

    set(LIBRARY_DEPS
        pixman
        freetype2
        fontconfig
    )

endif ()

#link the module like any other
target_link_libraries(${NAME} ${LIBRARY_DEPS})

add_post_build_command(cairo)

set(CAIRO_LIBRARY_HEADERS
    src/cairo.h
    src/cairo-deprecated.h
    src/cairo-features.h
    src/cairo-ft.h
    src/cairo-gl.h
    src/cairo-pdf.h
    src/cairo-ps.h
    src/cairo-svg.h
    src/cairo-version.h
    src/cairo-win32.h
    src/cairo-xml.h
)

copy_library_headers(cairo "${CAIRO_LIBRARY_HEADERS}" include/cairo)
