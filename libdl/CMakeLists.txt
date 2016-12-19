cmake_minimum_required(VERSION 2.8.12)

set(NAME "dl")
project(${NAME})

set(SOURCES dlfcn.c)

add_library(${NAME} STATIC ${SOURCES})
