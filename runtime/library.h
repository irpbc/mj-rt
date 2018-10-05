#ifndef MJ_RT_LIBRARY_H
#define MJ_RT_LIBRARY_H

#include <cstdint>
#include "Heap.h"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#ifdef _MSC_VER
#define ALLOC_CC __fastcall
#else
#define ALLOC_CC
#endif

extern "C" {

EXPORT void mjrt_init_runtime(uint8_t* LLVM_StackMaps);
EXPORT void* ALLOC_CC mjrt_alloc_impl(ObjectDescriptor* meta, uint64_t rsp);
EXPORT void* ALLOC_CC mjrt_alloc_array_impl(ObjectDescriptor* meta, int32_t length, uint64_t rsp);

EXPORT void mj_hello(void);
EXPORT int32_t mj_putchar(int32_t c);
EXPORT int32_t mj_puts(const char* str);
EXPORT int32_t mj_printf(const char* str, ...);

EXPORT int32_t mj_scan_int();
EXPORT int64_t mj_scan_long();
EXPORT float mj_scan_float();
EXPORT double mj_scan_double();

EXPORT int mj_getc();
EXPORT int mj_parseInt(ArrayBase* chars);
EXPORT char mj_intToChar(int32_t num);

}
#endif