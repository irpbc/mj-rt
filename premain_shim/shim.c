//
// Created by Ivan Rašić on 5.5.18..
//

#include <inttypes.h>

#ifdef __APPLE__
#include <mach-o/getsect.h>
#include <mach-o/ldsyms.h>
#endif

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include "pe_header.h"
#define DLL_IMPORT __declspec(dllimport)
#else
#define DLL_IMPORT
#endif

DLL_IMPORT void mjrt_init_runtime(uint8_t*);

void _premain() {

#ifdef __APPLE__
    unsigned long size = 0;
    uint8_t* stackmaps = getsectiondata(&_mh_execute_header, "__LLVM_STACKMAPS", "__llvm_stackmaps", &size);
#endif
#ifdef _WIN32
    void* stackmaps = getPESectionDataByName(".llvm_st");
#endif

    mjrt_init_runtime(stackmaps);
}
