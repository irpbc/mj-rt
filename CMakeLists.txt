cmake_minimum_required(VERSION 3.8)
project(mj_rt)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES
    lib/library.cpp
    lib/library.h
    lib/Heap.cpp
    lib/Heap.h
    lib/stack_shim.s
    statepoint_utils/statepoint.cpp
    statepoint_utils/statepoint.h
    lib/utils.h)

set_property(SOURCE lib/stack_shim.s PROPERTY LANGUAGE C)

add_library(mj_rt SHARED ${SOURCE_FILES})
add_library(mj_shim STATIC shim/shim.c)
