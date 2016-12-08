cmake_minimum_required(VERSION 2.8.12)

set(NAME "sqlite")
project(${NAME})

if (WIN32)

    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        amalgamation
        src/src
        "${CMAKE_SOURCE_DIR}/icu/source/common"
        "${CMAKE_SOURCE_DIR}/icu/source/i18n"
        "${CMAKE_SOURCE_DIR}/zlib"
    )

    set(INCLUDES
        amalgamation/sqlite3.h
        src/ext/fts2/fts2.h
        src/ext/fts2/fts2_hash.h
        src/ext/fts2/fts2_tokenizer.h
    )

    set(SOURCES
        amalgamation/sqlite3.c
        src/ext/fts2/fts2.c
        src/ext/fts2/fts2_hash.c
        src/ext/fts2/fts2_icu.c
        src/ext/fts2/fts2_porter.c
        src/ext/fts2/fts2_tokenizer.c
        src/ext/fts2/fts2_tokenizer1.c
    )

    add_definitions(-DU_DISABLE_RENAMING -DU_ICU_ENTRY_POINT_RENAME=1 -DSQLITE_CORE -DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_ICU -DSQLITE_ENABLE_MEMORY_MANAGEMENT -DSQLITE_SECURE_DELETE -DSQLITE_SEPARATE_CACHE_POOLS -DTHREADSAFE -DSQLITE_ENABLE_BROKEN_FTS2 -DSQLITE_ENABLE_FTS2 -DU_USING_ICU_NAMESPACE=0)

    add_library(${NAME} STATIC ${INCLUDES} ${SOURCES})

    add_post_build_command(sqlite)

    set(SQLITE_LIBRARY_HEADERS
        amalgamation/sqlite3.h
    )

    copy_library_headers(sqlite "${SQLITE_LIBRARY_HEADERS}" include)

else ()
    # -----------------------------------------------------------------------------
    # Add module directories
    # -----------------------------------------------------------------------------
    add_subdirectory(android)
    add_subdirectory(dist)
endif ()
