#include "library.h"

#include <stdio.h>


#define EXPORT __declspec(dllexport)

/**
 * Called as DllImport from .net to trigger library load.
 */
EXPORT void mj_init() {

}

EXPORT void mj_hello() {
    printf("Hello, World!\n");
}

EXPORT int mj_putchar(int c) {
    return putchar(c);
}

EXPORT int mj_puts(const char* str) {
    return puts(str);
}

EXPORT int mj_printf_int(const char* str, int arg) {
    return printf(str, arg);
}
