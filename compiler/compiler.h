#ifndef MJ_NATIVE_LIBRARY_H
#define MJ_NATIVE_LIBRARY_H

#include <llvm-c/Types.h>

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

extern "C" {

EXPORT void AddRewriteStatepointsForGCPass(LLVMPassManagerRef passManagerRef);

}


#endif