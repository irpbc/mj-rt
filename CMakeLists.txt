cmake_minimum_required(VERSION 3.8)
project(mj_rt)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

if(WIN32)
    enable_language(ASM_MASM)
    set(STACK_SHIM_SRC runtime/stack_shim_win.s runtime/stack_shim_win.def)
    set_property(SOURCE runtime/stack_shim_win.s PROPERTY LANGUAGE ASM_MASM)
else()
    set(STACK_SHIM_SRC runtime/stack_shim_nix.s)
	set_property(SOURCE ${STACK_SHIM_SRC} PROPERTY LANGUAGE C)
endif()

set(SOURCE_FILES
        runtime/library.cpp
        runtime/library.h
        runtime/Heap.cpp
        runtime/Heap.h
		${STACK_SHIM_SRC}
        statepoint_utils/statepoint.cpp
        statepoint_utils/statepoint.h
        runtime/utils.h)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})

add_library(mj_rt SHARED ${SOURCE_FILES})

set(SHIM_SRC premain_shim/shim.c)
if(WIN32)
	list(APPEND SHIM_SRC premain_shim/pe_header.h premain_shim/pe_header.c)
endif()
add_library(mj_shim STATIC ${SHIM_SRC})


set(COMP_NATIVE_SRC
        compiler/compiler.h
        compiler/compiler.cpp)

if(WIN32)
    add_library(libLLVM SHARED ${COMP_NATIVE_SRC} compiler/llvm.def compiler/dll_main.c)
    llvm_map_components_to_libnames(llvm_libs all)
    target_link_libraries(libLLVM ${llvm_libs})
	set(CMAKE_SHARED_LINKER_FLAGS "/OPT:NOREF /OPT:NOICF")
else()
    add_library(mj_compiler_native SHARED ${COMP_NATIVE_SRC})
    target_link_libraries(mj_compiler_native LLVM)
endif()
