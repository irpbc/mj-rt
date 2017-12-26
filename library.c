#include "library.h"

#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>


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
    scanf("%"PRId32, &x);
    return x;
}

EXPORT int64_t mj_scan_long() {
    int64_t x = 0L;
    scanf("%"PRId64 , &x);
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
