cmake_minimum_required(VERSION 2.8.12)

set(NAME "wtl")
project(${NAME})

add_custom_target(${NAME} ALL @xcopy /y /d /f \"${CMAKE_SOURCE_DIR}/wtl/include\" \"${WEBKIT_LIBRARIES_DIR}/include/wtl/\" >nul 2>nul)
