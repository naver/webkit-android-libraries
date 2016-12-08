set(FREETYPE_SOURCES
    src/autofit/autofit.c

    src/base/ftbase.c
    src/base/ftbbox.c
    src/base/ftbitmap.c
    src/base/ftfstype.c
    src/base/ftgasp.c
    src/base/ftglyph.c
    src/base/ftinit.c
    src/base/ftlcdfil.c
    src/base/ftstroke.c
    src/base/ftsystem.c
    src/base/fttype1.c
    src/base/ftxf86.c

    src/cff/cff.c

    src/pshinter/pshinter.c

    src/psnames/psnames.c

    src/raster/raster.c

    src/sfnt/sfnt.c

    src/smooth/smooth.c

    src/truetype/truetype.c
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
    builds
    include
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -fPIC -O3")
add_definitions(-DPIC -DDARWIN_NO_CARBON -DFT2_BUILD_LIBRARY -DTT_CONFIG_OPTION_BYTECODE_INTERPRETER)
add_library(freetype2 STATIC ${FREETYPE_SOURCES})

add_post_build_command(freetype2)

copy_library_headers_directory(freetype2 include include)
