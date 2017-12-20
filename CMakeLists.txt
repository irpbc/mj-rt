cmake_minimum_required(VERSION 3.8)
project(mj_rt)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES library.c library.h)
add_library(mj_rt SHARED ${SOURCE_FILES})