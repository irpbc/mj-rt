#include "library.h"

#include <stdio.h>
#include <stdarg.h>


EXPORT void mj_hello() {
    printf("Hello, World!\n");
}

EXPORT int mj_putchar(int c) {
    return putchar(c);
}

EXPORT int mj_puts(const char* str) {
    return puts(str);
}

EXPORT int mj_printf(const char* str, ...) {

    va_list args;
    va_start(args, str);
    return vprintf(str, args);
}