#include "library.h"

#include <cstdarg>

#ifdef _MSC_VER
#define ALLOC_CC __fastcall
#else
#define ALLOC_CC
#endif


Heap heap;

extern "C" {

EXPORT void mjrt_init_runtime(uint8_t* LLVM_StackMaps) {
    // 10 MB
    heap.initialize(10 * 1024 * 1024, LLVM_StackMaps);
}

EXPORT void* ALLOC_CC mjrt_alloc_impl(ObjectDescriptor* meta, uint64_t rsp) {
    void* previousFramePointer = (void*)rsp;
    return heap.allocateObject(meta, previousFramePointer);
}

EXPORT void* ALLOC_CC mjrt_alloc_array_impl(ObjectDescriptor* meta, int32_t length, uint64_t rsp) {
    void* previousFramePointer = (void*)rsp;
    return heap.allocateArray(meta, length, previousFramePointer);
}

EXPORT void mj_hello() {
    printf("Hello, World!\n");
}

EXPORT int32_t mj_putchar(int32_t c) {
    return putchar(c);
}

EXPORT int32_t mj_puts(const char* str) {
    return puts(str);
}

EXPORT int32_t mj_printf(const char* str, ...) {

    va_list args;
    va_start(args, str);
    return vprintf(str, args);
}

EXPORT int32_t mj_scan_int() {
    int32_t x = 0;
    scanf("%" PRId32, &x);
    return x;
}

EXPORT int64_t mj_scan_long() {
    int64_t x = 0L;
    scanf("%" PRId64, &x);
    return x;
}

EXPORT float mj_scan_float() {
    float x = 0.0f;
    scanf("%f", &x);
    return x;
}

EXPORT double mj_scan_double() {
    double x = 0.0;
    scanf("%lf", &x);
    return x;
}

}
