#ifndef MJ_RT_LIBRARY_H
#define MJ_RT_LIBRARY_H

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

EXPORT void mj_hello(void);
EXPORT int mj_putchar(int c);
EXPORT int mj_puts(const char* str);
EXPORT int mj_printf(const char* str, ...);

#endif