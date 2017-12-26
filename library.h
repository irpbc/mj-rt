#ifndef MJ_RT_LIBRARY_H
#define MJ_RT_LIBRARY_H

#include <stdint.h>

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

EXPORT void mj_hello(void);
EXPORT int32_t mj_putchar(int32_t c);
EXPORT int32_t mj_puts(const char* str);
EXPORT int32_t mj_printf(const char* str, ...);

EXPORT int32_t mj_scan_int();
EXPORT int64_t mj_scan_long();
EXPORT float mj_scan_float();
EXPORT double mj_scan_double();

#endif