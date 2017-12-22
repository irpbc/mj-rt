#ifndef MJ_RT_LIBRARY_H
#define MJ_RT_LIBRARY_H

#define EXPORT __declspec(dllexport)

EXPORT void mj_init();
EXPORT void mj_hello(void);
EXPORT int mj_putchar(int c);
EXPORT int mj_puts(const char* str);
EXPORT int mj_printf_int(const char* str, int arg);

#endif