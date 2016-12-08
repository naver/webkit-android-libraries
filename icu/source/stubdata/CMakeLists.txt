cmake_minimum_required(VERSION 2.8.12)

set(NAME "icudt")
project(${NAME})

set(SOURCES stubdata.c)

include_directories(
    "${CMAKE_SOURCE_DIR}/icu/source/common"
    "${CMAKE_SOURCE_DIR}/icu/source/i18n"
    "${CMAKE_SOURCE_DIR}/wtl/include"
)

add_definitions(-DU_DISABLE_RENAMING=1 -DU_HIDE_DATA_SYMBOL -DICU_BUILD=1 -DU_ICU_ENTRY_POINT_RENAME=1 -DU_USING_ICU_NAMESPACE=0 -DHAVE_DLOPEN=0 -DUCONFIG_NO_NON_HTML5_CONVERSION=1)

if (WIN32)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/CMakeGenerated/build")

    string(REPLACE "/" "\\" ICUDT_PATH "${CMAKE_SOURCE_DIR}/icu/windows/bin$(PlatformArchitecture)/icudt.dll")
    string(REPLACE "/" "\\" ICUDT_DEST "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>")

    add_custom_command(TARGET ${NAME}
                       POST_BUILD
                       COMMAND xcopy /y /u ${ICUDT_PATH} ${ICUDT_DEST})

    add_library(${NAME} SHARED ${SOURCES})

    set(LIBRARY_DEPS ${WIN32_SYSTEM_LIBRARIES})

    target_link_libraries(${NAME} ${LIBRARY_DEPS})
elseif (ANDROID)
    list(APPEND SOURCES
        ${CMAKE_SOURCE_DIR}/icu/android/icudtl_dat.S
    )

    enable_language(C ASM)
    set(ASM_OPTIONS "-x assembler-with-cpp")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${ASM_OPTIONS}")

    add_library(${NAME} STATIC ${SOURCES})
endif ()