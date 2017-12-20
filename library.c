#include "library.h"

#include <stdio.h>


/**
 * Called as DllImport from .net to trigger library load.
 */
void mj_init(void) {

}

void mj_hello(void) {
    printf("Hello, World!\n");
}

int mj_putchar(int c) {
    return putchar(c);
}

int mj_puts(const char* str) {
    return puts(str);
}